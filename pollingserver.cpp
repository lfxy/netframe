#include "pollingserver.h"
#include <iostream>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
//#include <sys/epoll.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>

using namespace std;

#define BUFFER_SIZE 10


PollingServer::PollingServer()
    : m_running(false),
      m_timeCheck(0),
      m_strTime(""),
      m_poll(new Polling),
      m_hasSendData(false),
      m_logBuf(new char[50])
{
    m_readTime = new timeStamp("read");
    m_convertTime = new timeStamp("convert");
    m_sendTime = new timeStamp("send");
    m_fileTime = new timeStamp("file");

}

PollingServer::~PollingServer()
{
    delete m_poll;
    delete[] m_logBuf;
    delete m_readTime;
    delete m_convertTime;
    delete m_sendTime;
    delete m_fileTime;
}

void PollingServer::Init(std::string& name, std::string ip, int port)
{
    m_servicName = name;
    m_serviceIp = ip;
    m_servicePort = port;
    m_running = true;
//    m_fileFd = fopen("./time.log", "a+");
    //m_outTimeFile.open("./time.log", std::ofstream::out | std::ofstream::app);
}


void PollingServer::Release()
{
    m_running = false;
    //fclose(m_fileFd);
    //m_outTimeFile.close();
}



void PollingServer::Run()
{
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, m_serviceIp.c_str(), &address.sin_addr);
    address.sin_port = htons(m_servicePort);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    ret = bind(listenfd, (struct sockaddr*) &address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    m_poll->AddFd(listenfd, EPOLLIN);
    m_setNonblocking(listenfd);
    while(m_running)
    {
        int ret = m_poll->Wait();
        if(ret < 0)
        {
            printf("epoll failure\n");
            break;
        }
    LtModel(ret, listenfd);
    }
    if(listenfd != -1)
    {
        close(listenfd);  
    }
}



int PollingServer::m_setNonblocking(int fd)
{
    int oldOption = fcntl(fd, F_GETFL);
    int newOption = oldOption | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOption);
    return oldOption;
}



char* PollingServer::m_getCurrentTime(std::string& id)
{

    time_t currtime;
    currtime = time(NULL);
    char* strTime = ctime(&currtime);
    //m_strTime = ctime(&currtime);
    //memset(m_logBuf, 0, 50 * sizeof(char));
    //m_outTimeFile.write(m_strTime.c_str(), m_strTime.size());
    int idsize = id.size();
    memcpy(m_logBuf, strTime, 24);
    memcpy(m_logBuf + 24, " ", sizeof(char));
    memcpy(m_logBuf + 25, id.c_str(), idsize);
    memcpy(m_logBuf + 25 + idsize, "\n", sizeof(char));
    //fwrite(m_logBuf, sizeof(char), 26 + idsize, m_fileFd);

//    fwrite(m_logBuf, sizeof(char), 25, m_fileFd);
    return NULL;
}

void PollingServer::m_handleListenFd(int listenfd)
{
    struct sockaddr_in client_address;
    socklen_t client_addrlen = sizeof(client_address);
    int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlen);
    m_timeCheck = 0;
    m_poll->AddFd(connfd, EPOLLIN);
    m_setNonblocking(connfd);
    //AddFd(epollfd, connfd, EPOLLIN | EPOLLOUT);
}

int PollingServer::m_handleReadfd(int sockfd)
{
    char buf[BUFFER_SIZE];
    memset(buf, '\0', BUFFER_SIZE);
    //int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
    int ret = read(sockfd, buf, BUFFER_SIZE - 1);
    if(ret > 0)
    {
        m_key = buf;
        if(m_serviceKey.count(m_key))
        {
            ++m_serviceKey[m_key];
        }
        else
        {
            m_serviceKey[m_key] = 1;
        }
        m_hasSendData = true;
        m_poll->ModFd(sockfd, EPOLLOUT);
        return 1;
    }
    else if(ret == 0)
    {
        printf("connect break!\n");
        return 0;
    }
    else if(ret == -1)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            printf("read return with EAGAIN\n");
            return -1;
        }
        else
        {
            printf("read return with errno:%d\n", errno);
            m_poll->RemoveFd(sockfd);
            close(sockfd);
            return -2;
        }

    }
    else
    {
        printf("read return with result:%d\n", ret);
        m_poll->RemoveFd(sockfd);
        close(sockfd);
        return -3;
    }
}


int PollingServer::m_handleSendFd(int sockfd)
{
//    if(m_hasSendData)
    {
        std::string& v = m_convertStr.GetString(m_serviceKey[m_key]);
        int ret = write(sockfd, v.c_str(), v.size());
        static unsigned int write_cnt = 0;
        if(write_cnt == 0)
        {
            m_sendTime->StartRecord();
        }
        if(write_cnt++ % 100000 == 0){
            m_sendTime->EndRecord();
            m_sendTime->StartRecord();
            printf("write events fd[%d], write_cnt[%u]\n", sockfd, write_cnt);
        }
        m_hasSendData = false;
        if(ret > 0)
        {
            m_poll->ModFd(sockfd, EPOLLIN);
            return 1;
        }
        else if(ret == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                printf("send EAGAIN error!!!!\n");
                return -1;
            }
            else
            {
                printf("send other errno: %d\n", errno);
                m_poll->RemoveFd(sockfd);
                close(sockfd);
                return -2;
            }
        }
        else
        {
            printf("send other result: %d\n", ret);
            m_poll->RemoveFd(sockfd);
            close(sockfd);
            return -3;

        }
    }
}

void PollingServer::LtModel(int number, int listenfd)
{

    for(int i = 0; i < number; ++i)
    {
        int sockfd = m_poll->GetActiveFd(i);
        int event = m_poll->GetActiveEvent(i);
        if(sockfd == listenfd)
        {
            m_handleListenFd(listenfd);
        }
        else if(CanRead(event))
        {
            if(m_handleReadfd(sockfd) == 0)
            {
                m_poll->RemoveFd(sockfd);
            }
        }
        else if(CanWrite(event))
        {
            m_handleSendFd(sockfd);
        }
        else
        {
            printf("something else happened!\n");
        }
    }

}


/*void PollingServer::EtModel(epoll_event* events, int number, int epollfd, int listenfd)
{
    char buf[ BUFFER_SIZE ];
    for ( int i = 0; i < number; i++ )
    {
        int sockfd = events[i].data.fd;
        if ( sockfd == listenfd )
        {
            struct sockaddr_in client_address;
            socklen_t client_addrlength = sizeof( client_address );
            int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );
            AddFd( epollfd, connfd, true, false, true, true);
        }
        else if ( events[i].events & EPOLLIN )
        {
            while( 1 )
            {
                memset( buf, '\0', BUFFER_SIZE );
                int ret = recv( sockfd, buf, BUFFER_SIZE-1, 0 );
                if( ret < 0 )
                {
                    if( ( errno == EAGAIN ) || ( errno == EWOULDBLOCK ) )
                    {
                        break;
                    }
                    close(sockfd);
                    break;
                }
                else if( ret == 0 )
                {
                    close(sockfd);
                }
                else
                {
                    //printf( "get %d bytes of content: %s\n", ret, buf );
                }
            }
            m_key = buf;
            if(m_serviceKey.count(m_key))
            {
                m_serviceKey[m_key]++;
            }
            else
            {
                m_serviceKey[m_key] = 1;
            }
            AddFd(epollfd, sockfd, false, true, false, true);
        }
        else if(events[i].events & EPOLLOUT)
        {
            std::string& v = m_convertStr.GetString(m_serviceKey[m_key]);
            send(sockfd, v.c_str(), v.size(), 0);
            AddFd(epollfd, sockfd, true, false, false, true);
        }
        else
        {
            //printf( "something else happened \n" );
        }
    }
}

void  PollingServer::AddFd(int epollfd, int fd, bool enableread, bool enablewrite, bool bfirst, bool enable_et)
{
    epoll_event event;
    event.data.fd = fd;
    if(enableread)
    {
        event.events = EPOLLIN;
    }
    if(enablewrite)
    {
        event.events = EPOLLOUT;
    }
    if(enable_et)
    {
        event.events |= EPOLLET;
    }

    if(bfirst)
    {
        epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
        m_setNonblocking(fd);
    }
    else
    {
        epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
    }
}*/


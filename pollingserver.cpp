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

using namespace std;

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10


PollingServer::PollingServer()
    : m_running(false),
      m_timeCheck(0)
{

}

PollingServer::~PollingServer()
{

}

void PollingServer::Init(std::string& name, std::string ip, int port)
{
    m_servicName = name;
    m_serviceIp = ip;
    m_servicePort = port;
    m_running = true;
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

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    AddFd(epollfd, listenfd, true, false, true, true);
    while(m_running)
    {
        int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        //printf("After epoll_wait here\n");
        if(ret < 0)
        {
            printf("epoll failure\n");
            break;
        }
    LtModel(events, ret, epollfd, listenfd);
    }
    close(listenfd);
}


void PollingServer::Release()
{
    m_running = false;
}




int PollingServer::m_setNonblocking(int fd)
{
    int oldOption = fcntl(fd, F_GETFL);
    int newOption = oldOption | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOption);
    return oldOption;
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
}


void PollingServer::LtModel(epoll_event* events, int number, int epollfd, int listenfd)
{
    char buf[BUFFER_SIZE];

    for(int i = 0; i < number; ++i)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd)
        {
            //printf("Listenfd LtModel here\n");
            struct sockaddr_in client_address;
            socklen_t client_addrlen = sizeof(client_address);
            int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlen);
            m_timeCheck = 0;
            AddFd(epollfd, connfd, true, false, true, true);
        }
        else if(events[i].events & EPOLLIN)
        {
            //printf("Read event trigger once\n");
            memset(buf, '\0', BUFFER_SIZE);
            int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
            if(ret <= 0)
            {
                close(sockfd);
                continue;
            }
            //printf("Get %d bytes of content:%s\n", ret, buf);
            m_key = buf;
            if(m_serviceKey.count(m_key))
            {
                m_serviceKey[m_key]++;
            }
            else
            {
                m_serviceKey[m_key] = 1;
            }
            AddFd(epollfd, sockfd, false, true, false, false);
        }
        else if(events[i].events & EPOLLOUT)
        {
            //printf("Write event tirgger %d\n", m_serviceKey[m_key]);
            if(m_timeCheck == 0)
            {
                printf("Start record!\n");
                m_timeGet.StartRecord();
            }
            std::string& v = m_convertStr.GetString(m_serviceKey[m_key]);
            send(sockfd, v.c_str(), v.size(), 0);
            if(++m_timeCheck >= 10000)
            {
                m_timeGet.EndRecord();
                printf("End record!\n");
                m_timeCheck = 0;
                printf("Record time is:%ld\n", m_timeGet.GetTime());
            }
            AddFd(epollfd, sockfd, true, false, false, false);
        }
        else
        {
            //printf("something else happened!\n");
        }
    }

}


void PollingServer::EtModel(epoll_event* events, int number, int epollfd, int listenfd)
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
            //printf( "event trigger once\n" );
            while( 1 )
            {
                memset( buf, '\0', BUFFER_SIZE );
                int ret = recv( sockfd, buf, BUFFER_SIZE-1, 0 );
                if( ret < 0 )
                {
                    if( ( errno == EAGAIN ) || ( errno == EWOULDBLOCK ) )
                    {
                        //printf( "read later\n" );
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
            //printf("Write event tirgger :%d\n", m_serviceKey[m_key]);
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

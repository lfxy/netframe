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
//#include <sys/epoll.h>
#include <pthread.h>


using namespace std;

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10


PollingServer::PollingServer()
{

}

PollingServer::~PollingServer()
{

}

void PollingServer::Init(std::string& name)
{
    m_ServicName = name;
}


void PollingServer::Run()
{


}

void PollingServer::Release()
{

}




int PollingServer::m_setNonblocking(int fd)
{
    int oldOption = fcntl(fd, F_GETFL);
    int newOption = oldOption | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOption);
    return oldOption;
}


void  PollingServer::AddFd(int epollfd, int fd, bool enable_et)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLOUT;
    if(enable_et)
    {
        event.events |= EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    m_setNonblocking(fd);
}


void PollingServer::LtModel(epoll_event* events, int number, int epollfd, int listenfd)
{
    char buf[BUFFER_SIZE];

    for(int i = 0; i < number; ++i)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd)
        {
            printf("LtModel listenfd here\n");
            struct sockaddr_in client_address;
            socklen_t client_addrlen = sizeof(client_address);
            int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlen);
            AddFd(epollfd, connfd, false);
        }
        else if(events[i].events & EPOLLIN)
        {
            printf("Read event trigger once\n");
            memset(buf, '\0', BUFFER_SIZE);
            int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
            if(ret <= 0)
            {
                close(ret <= 0);
                continue;
            }
            printf("Get %d bytes of content:%s\n", ret, buf);
            m_key = buf;
            if(m_serviceKey.count(m_key))
            {
                m_serviceKey[m_key]++;
                send(sockfd, &m_serviceKey[m_key], sizeof(int), 0);
            }
            else
            {
                m_serviceKey[m_key] = 1;
            }
        }
        else if(events[i].events & EPOLLOUT)
        {
            printf("Write event tirgger.\n");
            if(m_serviceKey.count(m_key))
            {
                m_serviceKey[m_key]++;
                send(sockfd, &m_serviceKey[m_key], sizeof(int), 0);
            }
            else
            {
                m_serviceKey[m_key] = 1;
            }
        }
    }

}



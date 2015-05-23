#include "Polling.h"
#include <unistd.h>
#include <assert.h>


Polling::Polling()
{
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);
}

Polling::~Polling()
{
    if(m_epollfd != -1)
    {
        close(m_epollfd);
    }

}


int Polling::AddFd(int fd, int ev)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = ev;
    return epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &event);
}


int Polling::RemoveFd(int fd)
{
    epoll_event event;
    return epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &event);
}

    
int Polling::ModFd(int fd, int ev)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = ev;
    return epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &event);
}

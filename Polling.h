#ifndef _POLLING_H_
#define _POLLING_H_

#include <sys/epoll.h>
#include <netinet/in.h>

class Polling
{
public:
    Polling();
    ~Polling();

    int AddFd(int fd, int ev);
    int RemoveFd(int fd);
    int ModFd(int fd, int ev);

    int AddReadFd(int fd){return AddFd(fd, EPOLLIN);}
    int AddWriteFd(int fd){return AddFd(fd, EPOLLOUT);}
    int AddReadWriteFd(int fd){return AddFd(fd, EPOLLIN | EPOLLOUT);}
    int ModReadFd(int fd){return ModFd(fd, EPOLLIN);}
    int ModWriteFd(int fd){return ModFd(fd, EPOLLOUT);}
    int ModReadWriteFd(int fd){return ModFd(fd, EPOLLIN | EPOLLOUT);}


    int Wait(int timeout_ms = WAIT_TIMEOUT_MS){return epoll_wait(m_epollfd,  m_events, MAX_EVENT_NUMBER, timeout_ms);}
    #define GetActiveFd(idx)    m_events[idx].data.fd 
    #define GetActiveEvent(idx) m_events[idx].events
    #define CanRead(event)      (event & EPOLLIN)
    #define CanWrite(event)     (event & EPOLLOUT)

public:
    int m_epollfd;
    static const int MAX_EVENT_NUMBER = 1024;
    epoll_event m_events[MAX_EVENT_NUMBER];
    static const int WAIT_TIMEOUT_MS = 1000;

};



#endif

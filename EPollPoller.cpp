#include "EPollPoller.h"
#include "Handler.h"
#include <boost/static_assert.hpp>
#include <assert.h>
#include <errno.h>
#include <sys/epoll.h>
#include <stdio.h>


const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop),
      m_epollfd(::epoll_create1(EPOLL_CLOEXEC)),
      m_eventlist(kInitEventListSize)
{
    if(m_epollfd < 0)
    {
        printf("epoll_create1 error!\n");
    }
}

EPollPoller::~EPollPoller()
{
    ::close(m_epollfd);
}

void EPollPoller::PollEvent(int timeoutMs, HandlerList* activeHandles)
{
    int numEvents = ::epoll_wait(m_epollfd, &*m_eventlist.begin(), static_cast<int>(m_eventlist.size()), timeoutMs);
    if(numEvents > 0)
    {
        fillActiveHandlers(numEvents, activeHandles);
        if(numEvents == static_cast<int>(m_eventlist.size()))
        {
            m_eventlist.resize(m_eventlist.size() * 2);
        }
    }
    else if(numEvents == 0)
    {
        printf("PollEvent nothing happened!\n");
    }
    else
    {
        printf("PollEvent error!\n");
    }
}

void EPollPoller::updateHandler(Handler* handler)
{
    Poller::assertInLoopThread();
    const int state = handler->getState();
    int fd = handler->getFd();
    if(state == kNew || state == kDeleted)
    {
        if(state == kNew)
        {
            assert(m_handlermap.find(fd) == m_handlermap.end());
            m_handlermap[fd] = handler;
        }
        else
        {
            assert(m_handlermap.find(fd) != m_handlermap.end());
            assert(m_handlermap[fd] == handler);
        }
        handler->setState(kAdded);
        update(EPOLL_CTL_ADD, handler);
    }
    else
    {
        assert(m_handlermap.find(fd) != m_handlermap.end());
        assert(m_handlermap[fd] == handler);
        assert(handler->getState() == kAdded);
        if(handler->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, handler);
            handler->setState(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, handler);
        }
    }
}

void EPollPoller::removeHandler(Handler* handler)
{
    Poller::assertInLoopThread();
    int fd = handler->getFd();
    int state = handler->getState();
    assert(m_handlermap.find(fd) != m_handlermap.end());
    assert(m_handlermap[fd] == handler);
    assert(handler->isNoneEvent());
    assert(state == kAdded || state == kDeleted);
    m_handlermap.erase(fd);
    if (state == kAdded)
    {
        update(EPOLL_CTL_DEL, handler);
    }
    handler->setState(kNew);
}


void EPollPoller::fillActiveHandlers(int numEvents, HandlerList* activeHandles) const
{
    assert(numEvents <= static_cast<int>(m_handlermap.size()));
    for(int i = 0; i < numEvents; ++i)
    {
        Handler* handler = static_cast<Handler*>(m_eventlist[i].data.ptr);
        //int fd = handler.fd();
        //HandlerMap::const_iterator it = m_handlermap.find(fd);
        //assert(it != m_handlermap.end());
        //assert(it->second == handler);
        handler->setRevents(m_eventlist[i].events);
        activeHandles->push_back(handler);

    }

}

void EPollPoller::update(int operation, Handler* handler)
{
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = handler->getEvents();
    event.data.ptr = handler;
    int fd = handler->getFd();
    if(::epoll_ctl(m_epollfd, operation, fd, &event) < 0)
    {
        printf("EPollPoller::update epoll_ctl operation:%d, fd:%d\n", operation, fd);
    }
}

#include "Handler.h"
#include <poll.h>
#include <sys/epoll.h>
#include "EventLoop.h"

const int Handler::kNoneEvent = 0;
const int Handler::kReadEvent = EPOLLIN | EPOLLPRI;
const int Handler::kWriteEvent = EPOLLOUT;

Handler::Handler(EventLoop* loop, int fd)
    : m_pLoop(loop),
      m_fd(fd),
      m_events(0),
      m_revents(0),
      m_index(0),
      m_btied(false),
      m_bEventHandling(false)
{

}
Handler::~Handler()
{
    assert(!m_bEventHandling);
}


void Handler::m_updateHandler()
{
    m_pLoop->updateHandler(this);
}


void Handler::removeHandler()
{
    assert(isNoneEvent());
    m_pLoop->removeHandler(this);
}

void Handler::handleEvent()
{
    boost::shared_ptr<void> guard;
    if(m_btied)
    {
        guard = m_tie.lock();
        if(guard)
        {
            m_handleEvent();
        }
    }
    else
    {
        m_handleEvent();
    }

}

void Handler::m_handleEvent()
{
    m_bEventHandling = true;
    if((m_revents & EPOLLHUP) && !(m_revents & EPOLLIN))
    {
        if(m_closeCallback) m_closeCallback();
    }
    if(m_revents & (EPOLLERR | POLLNVAL))
    {
        if(m_errorCallback) m_errorCallback();
    }
    if(m_revents & (EPOLLIN | POLLPRI | EPOLLRDHUP))
    {
        if(m_readCallback) m_readCallback();
    }
    if(m_revents & EPOLLOUT)
    {
        if(m_writeCallback) m_writeCallback();
    }
    m_bEventHandling = false;
}


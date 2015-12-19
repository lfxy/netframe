#include "Handler.h"
#include <poll.h>
#include <sys/epoll.h>

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

}

void Handler::handleEvent()
{

}

void Handler::removeHandler()
{

}

void Handler::m_updateHandler()
{

}
void Handler::m_handleEvent()
{

}

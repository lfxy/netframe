#include "EventLoop.h"
#include "Mutex.h"
#include "Handler.h"
#include "Poller.h"
//#include "SocketsOps.h"
//#include "TimerQueue.h"

#include <boost/bind.hpp>
#include <signal.h>
#include <sys/eventfd.h>

__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0)
    {
        abort();
    }
    return evtfd;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

EventLoop::EventLoop()
    : m_blooping(false),
      m_bquit(false),
      m_beventHandling(false),
      m_callingPendingFunctors(false),
      m_threadId(CurrentThread::tid()),
      m_poller(Poller::newDefaultPoller(this)),
      m_wakeupFd(createEventfd()),
      m_wakeupHandler(new Handler(this, m_wakeupFd)),
      m_currentActiveHandler(NULL)
{
    if(!t_loopInThisThread)
    {
        t_loopInThisThread = this;
    }
    else
    {
        printf("Another EventLoop exists!\n");
    }
    m_wakeupHandler->setReadCallback(boost::bind(&EventLoop::m_handleRead, this));
    m_wakeupHandler->enableReading();
}

EventLoop::~EventLoop()
{
    ::close(m_wakeupFd);
    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    assert(!m_blooping);
    assertInLoopThread();
    m_blooping = true;
    m_bquit = false;
    while(!m_bquit)
    {
        m_activeHandlers.clear();
        m_poller->PollEvent(kPollTimeMs, &m_activeHandlers);
        m_beventHandling = true;
        for(HandlerList::iterator it = m_activeHandlers.begin(); it != m_activeHandlers.end(); ++it)
        {
            m_currentActiveHandler = *it;
            m_currentActiveHandler->handleEvent();
        }
        m_currentActiveHandler = NULL;
        m_beventHandling = false;
        m_doPendingFunctors();
    }
    m_blooping = false;
}


void EventLoop::quit()
{
    m_bquit = true;
    if(!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(const Functor& cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        printf("EventLoop::runInLoop not in loop thread.\n");
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        MutexLockGuard lock(m_mutex);
        m_pendingFuntors.push_back(cb);
    }
    if(!isInLoopThread() || m_callingPendingFunctors)
    {
        wakeup();
    }
}

void EventLoop::updateHandler(Handler* handle)
{
    assert(handle->ownerLoop() == this);
    assertInLoopThread();
    m_poller->updateHandler(handle);
}

void EventLoop::removeHandler(Handler* handle)
{
    assert(handle->ownerLoop() == this);
    assertInLoopThread();
    if(m_beventHandling)
    {
        assert(m_currentActiveHandler == handle || std::find(m_activeHandlers.begin(), m_activeHandlers.end(), handle) == m_activeHandlers.end());
    }
    m_poller->removeHandler(handle);
}

void EventLoop::m_abortNotInLoopThread()
{

}

void EventLoop::wakeup()
{
    //uint64_t one = 1;
    /*ssize_t n = sockets::write(m_wakeupFd, &one, sizeof(one));
    if(n != sizeof(one))
    {
        printf("wakeup %d bytes\n", n);
    }*/
}

void EventLoop::m_handleRead()
{
    //uint64_t one = 1;
    /*ssize_t n = sockets::read(m_wakeupFd, &one, sizeof(one));
    if(n != sizeof(one))
    {
        printf("wakeup %d bytes\n", n);
    }*/
}

void EventLoop::m_doPendingFunctors()
{
    std::vector<Functor> tmpFunctors;
    m_callingPendingFunctors = true;
    {
        MutexLockGuard lock(m_mutex);
        tmpFunctors.swap(m_pendingFuntors);
    }

    for(size_t i = 0; i < tmpFunctors.size(); i++)
    {
        tmpFunctors[i]();
    }
    m_callingPendingFunctors = false;
}

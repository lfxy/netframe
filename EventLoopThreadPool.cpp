#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <boost/bind.hpp>


EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop)
    : m_baseLoop(baseloop),
      m_started(false),
      m_threadsNum(0),
      m_next(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    assert(!m_started);
    m_baseLoop->assertInLoopThread();
    m_started = true;
    for(int i = 0; i < m_threadsNum; ++i)
    {
        EventLoopThread* t = new EventLoopThread(cb);
        m_threads.push_back(t);
        m_loops.push_back(t->startLoop());
    }
    if(m_threadsNum == 0 && cb)
    {
        cb(m_baseLoop);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    m_baseLoop->assertInLoopThread();
    EventLoop* loop = m_baseLoop;

    if(!m_loops.empty())
    {
        loop = m_loops[m_next];
        ++m_next;
        if((size_t)m_next >= m_loops.size())
        {
            m_next = 0;
        }
    }
    return loop;
}

#include "EventLoopThread.h"
#include "EventLoop.h"
#include <boost/bind.hpp>

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb)
    : m_ploop(NULL),
    m_bexiting(false),
    m_thread(boost::bind(&EventLoopThread::threadFunc, this)),
    m_mutex(),
    m_cond(m_mutex),
    m_callback(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    m_bexiting = true;
    m_ploop->quit();
    printf("EventLoopThread ~~~\n");
    m_thread.join();
}

//run in old thread
EventLoop* EventLoopThread::startLoop()
{
    assert(!m_thread.started());
    m_thread.start();
    printf("EventLoopThread::startLoop after start\n");

    {
        MutexLockGuard lock(m_mutex);
        printf("EventLoopThread::startLoop after start 111\n");
        while(m_ploop == NULL)
        {
            printf("EventLoopThread::startLoop after start 222\n");
            m_cond.wait();
            printf("EventLoopThread::startLoop after start 333\n");
        }
    }
    return m_ploop;
}

//run in new thread
void EventLoopThread::threadFunc()
{
    EventLoop loop;
    if(m_callback)
    {
        m_callback(&loop);
    }
    {
        MutexLockGuard lock(m_mutex);
        m_ploop = &loop;
        m_cond.notify();
    }
    printf("EventLoopThread::threadFunc 111111\n");
    loop.loop();
    printf("EventLoopThread::threadFunc 222222\n");
}

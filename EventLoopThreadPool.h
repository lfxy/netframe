#ifndef _EVENTLOOP_THREAD_POOL_H
#define _EVENTLOOP_THREAD_POOL_H

#include "Condition.h"
#include "Mutex.h"

#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : boost::noncopyable
{
    public:
        typedef boost::function<void(EventLoop*)> ThreadInitCallback;
        EventLoopThreadPool(EventLoop* baseloop);
        ~EventLoopThreadPool();
        void setThreadNum(int threadnum){ m_threadsNum = threadnum; }
        void start(const ThreadInitCallback& cb = ThreadInitCallback());
        EventLoop* getNextLoop();

    private:
        EventLoop* m_baseLoop;
        bool m_started;
        int m_threadsNum;
        int m_next;
        boost::ptr_vector<EventLoopThread> m_threads;
        std::vector<EventLoop*> m_loops;
};

#endif

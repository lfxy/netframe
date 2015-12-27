#ifndef _NET_EVENTLOOP_THREAD_H_
#define _NET_EVENTLOOP_THREAD_H_

#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"
#include <boost/noncopyable.hpp>

class EventLoop;

class EventLoopThread : boost::noncopyable
{
    public:
        typedef boost::function<void(EventLoop*)> ThreadInitCallback;

        EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
        ~EventLoopThread();
        EventLoop* startLoop();

    private:
        void threadFunc();

        EventLoop* m_ploop;
        bool m_bexiting;
        Thread m_thread;
        MutexLock m_mutex;
        Condition m_cond;
        ThreadInitCallback m_callback;
};


#endif

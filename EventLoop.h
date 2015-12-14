#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <Mutex.h>
#include <Thread.h>
#include <Handler.h>
#include <vector>

class Handler;
//class Poller;

class EventLoop : boost::noncopyable
{
public:
    typedef boost::function<void()> Functor;
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();

    void runInLoop(const Functor& cb);
    void queueInLoop(const Functor& cb);

    void wakeup();
    void updateHandler(Handler* handler);
    void removeHandler(Handler* handler);

    void assertInLoopThread()
    {
        if(!isInLoopThread()){
            //abortNotInLoopThread();
            //assert();
        }
    }
    bool isInLoopThread() const 
    {
        return m_threadId == CurrentThread::tid();
    }

    static EventLoop* getEventLoopOfCurrentThread();
private:
    void m_abortNotInLoopThread();
    void m_handleRead();
    void m_doPendingFunctors();

    typedef std::vector<Handler*> HandlerList;
    bool m_blooping;
    bool m_bquit;
    bool m_beventHandling;
    bool m_callingPendingFunctors;
    const pid_t m_threadId;
    //boost::scoped_ptr<Poller> m_poller;
    int m_wakeupFd;
    boost::scoped_ptr<Handler> m_wakeupHandler;
    HandlerList m_activeHandlers;
    Handler* m_currentActiveHandler;
    MutexLock m_mutex;
    std::vector<Functor> m_pendingFuntors;
};

#endif

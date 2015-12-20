#ifndef _NET_POLLER_H_
#define _NET_POLLER_H_
#include <vector>
#include <boost/noncopyable.hpp>
#include "EventLoop.h"

class Handler;

class Poller : boost::noncopyable
{
    public:
        typedef std::vector<Handler*> HandlerList;
        Poller(EventLoop* loop);
        virtual ~Poller();
        virtual void poll(int timeMs, HandlerList* activeChannels) = 0;
        virtual void updateHandler(Handler* handler) = 0;
        virtual void removeHandler(Handler* handler) = 0;

        static Poller* newDefaultPoller(EventLoop* loop);
        void asserInLoopThread()
        {
            m_pLoop->assertInLoopThread();
        }
    private:
        EventLoop* m_pLoop;
};

#endif

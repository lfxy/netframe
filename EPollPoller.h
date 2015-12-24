#ifndef _NET_EPOLL_POLLER_H
#define _NET_EPOLL_POLLER_H

#include <map>
#include <vector>
#include "Poller.h"

struct epoll_event;

class EPollPoller : public Poller
{
    public:
        EPollPoller(EventLoop* loop);
        virtual ~EPollPoller();

        virtual void PollEvent(int timeoutMs, HandlerList* activeHandles);
        virtual void updateHandler(Handler* handler);
        virtual void removeHandler(Handler* channel);

    private:
        static const int kInitEventListSize = 16;
        void fillActiveHandlers(int numEvents, HandlerList* activeHandles) const;
        void update(int operation, Handler* handler);

        typedef std::vector<struct epoll_event> EventList;
        typedef std::map<int, Handler*> HandlerMap;

        int m_epollfd;
        EventList m_eventlist;
        HandlerMap m_handlermap;
};

#endif

#ifndef _HANDLER_H_
#define _HANDLER_H_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class EventLoop;

class Handler : boost::noncopyable
{
public:
    typedef boost::function<void()> EventCallback;
    //typedef boost::function<void(Timestamp)> ReadCallback;

    Handler(EventLoop* loop, int fd);
    ~Handler();

    void handleEvent();
    void setReadCallback(const EventCallback& cb){m_readCallback = cb;}
    void setWriteCallback(const EventCallback& cb){m_writeCallback = cb;}
    void setCloseCallback(const EventCallback& cb){m_closeCallback = cb;}
    void setErrorCallback(const EventCallback& cb){m_errorCallback = cb;}
    void setTie(const boost::shared_ptr<void>& obj){m_tie = obj; m_btied = true;}
    int getFd() const {return m_fd;}
    int getEvents() const {return m_events;}
    void setEvents(int revt) {m_revents = revt;}
    bool isNoneEvent() const {return m_events == kNoneEvent;}
    void enableReading() {m_events |= kReadEvent; m_updateHandler();}
    void enableWriting() {m_events |= kWriteEvent; m_updateHandler();}
    void disableWriting() {m_events &= ~kWriteEvent; m_updateHandler();}
    bool isWriting() const {return m_events & kWriteEvent;}

    int getIndex(){return m_index;}
    void setIndex(int idx){m_index = idx;}
    EventLoop* ownerLoop(){return m_pLoop;}
    void removeHandler();

private:
    void m_updateHandler();
    void m_handleEvent();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* m_pLoop;
    const int m_fd;
    int m_events;
    int m_revents;
    int m_index;

    boost::weak_ptr<void> m_tie;
    bool m_btied;
    bool m_bEventHandling;

    EventCallback m_readCallback;
    EventCallback m_writeCallback;
    EventCallback m_closeCallback;
    EventCallback m_errorCallback;
};

#endif

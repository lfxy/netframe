#ifndef _NET_ACCEPTOR_H_
#define _NET_ACCEPTOR_H_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "Handler.h"
#include "Socket.h"

class EventLoop;
class InetAddress;

class Acceptor : boost::noncopyable
{
    public:
        typedef boost::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;
        Acceptor(EventLoop* loop, const InetAddress& listenAddr);
        ~Acceptor();

        void setNewConnectionCallback(const NewConnectionCallback& cb){m_newConnectionCb = cb;}
        bool listenning() const {return m_blistenning;}
        void listen();

    private:
        void handleRead();

        EventLoop* m_ploop;
        Socket m_acceptSocket;
        Handler m_acceptHandler;
        NewConnectionCallback m_newConnectionCb;
        bool m_blistenning;
        int m_idleFd;
};

#endif

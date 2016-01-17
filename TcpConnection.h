#ifndef _NET_TCPCONNECTION_H_
#define _NET_TCPCONNECTION_H_

#include "Mutex.h"
#include "Buffer.h"
#include "InetAddress.h"
#include "Callbacks.h"

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

class Handler;
class EventLoop;
class Socket;


class TcpConnection : boost::noncopyable, public boost::enable_shared_from_this<TcpConnection>
{
    public:
        TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr);
        ~TcpConnection();

        EventLoop* getLoop() const { return m_ploop; }
        const std::string& getName() const { return m_name; }
        const InetAddress& getLocalAddress() { return m_localAddr; }
        const InetAddress& getPeerAddress() { return m_peerAddr; }
        bool isConnected() const { return m_state == kConnected; }
        void send(const void* message, size_t len);
        void send(const std::string& message);
        void send(Buffer* message);
        void shutdown(); //not thread safe
        void setTcpNoDelay(bool on);

        void setContext(const boost::any& context){ m_context = context; }
        const boost::any& getContext() const { return m_context; }

        void setConnectionCallback(const ConnectionCallback& cb) { m_connectionCB = cb; }
        void setMessageCallback(const MessageCallback& cb) { m_messageCB = cb; }
        void setWriteCompleteCallback(const WriteCompleteCallback& cb) { m_writeCompleteCB = cb; }
        void setHighWaterMakrCallback(const HighWaterCallback& cb, size_t highWaterMark) { m_highWaterCB = cb; m_highWaterMark = highWaterMark; }

        Buffer* getInputBuffer() { return &m_inputBuffer; }
        void setCloseCallback(const CloseCallback& cb) { m_closeCB = cb; }
        void connectionEstablished();
        void connectionDestroyed();

    private:
        enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
        void handleRead(/* TimeStamp receiveTime */);
        void handleWrite();
        void handleClose();
        void handleError();
        void sendInLoop(const std::string& message);
        void sendInLoop(const void* data, size_t len);
        void shutdownInLoop();
        void setState(StateE s) { m_state = s; }

        EventLoop* m_ploop;
        std::string m_name;
        StateE m_state;
        boost::scoped_ptr<Socket> m_socket;
        boost::scoped_ptr<Handler> m_handler;
        InetAddress m_localAddr;
        InetAddress m_peerAddr;
        ConnectionCallback m_connectionCB;
        MessageCallback m_messageCB;
        WriteCompleteCallback m_writeCompleteCB;
        HighWaterCallback m_highWaterCB;
        CloseCallback m_closeCB;
        size_t m_highWaterMark;
        Buffer m_inputBuffer;
        Buffer m_outputBuffer;
        boost::any m_context;
};
typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
#endif

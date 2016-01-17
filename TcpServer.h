#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include "TcpConnection.h"
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : boost::noncopyable
{
    public:
        typedef boost::function<void(EventLoop*)> ThreadInitCallback;

        TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg);
        ~TcpServer();

        const std::string& getHostPort() const { return m_hostPort; }
        const std::string& getName() const { return m_name; }
        void setThreadNum(int threadsNum);
        void setThreadInitCallback(const ThreadInitCallback& cb) { m_threadInitCB = cb; }
        void start();
        void setConnectionCallback(const ConnectionCallback& cb) { m_connectionCB = cb; }
        void setMessageCallback(const MessageCallback& cb) { m_messageCB = cb; }
        void setWriteCompleteCallback(const WriteCompleteCallback& cb) { m_writeCompleteCB = cb; }

    private:
        void newConnection(int sockfd, const InetAddress& peerAddr);
        void removeConnection(const TcpConnectionPtr& conn);
        void removeConnectionInLoop(const TcpConnectionPtr& conn);

        typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

        EventLoop* m_ploop;
        const std::string m_hostPort;
        const std::string m_name;
        const boost::scoped_ptr<Acceptor> m_acceptor;
        const boost::scoped_ptr<EventLoopThreadPool> m_threadPool;
        ConnectionCallback m_connectionCB;
        MessageCallback m_messageCB;
        WriteCompleteCallback m_writeCompleteCB;
        ThreadInitCallback m_threadInitCB;
        bool m_started;
        int m_nextConnId;
        ConnectionMap m_connections;
};

#endif

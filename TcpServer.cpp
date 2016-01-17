#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketOps.h"

#include <boost/bind.hpp>
#include <stdio.h>

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameArg)
    : m_ploop(loop),
      m_hostPort(listenAddr.toIpPort()),
      m_acceptor(new Acceptor(loop, listenAddr)),
      m_threadPool(new EventLoopThreadPool(loop)),
      m_connectionCB(defaultConnectionCB),
      m_messageCB(defaultMessageCB),
      m_started(false),
      m_nextConnId(1)
{
    m_acceptor->setNewConnectionCallback(boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    m_ploop->assertInLoopThread();

    for(ConnectionMap::iterator it(m_connections.begin()); it != m_connections.end(); ++it)
    {
        TcpConnectionPtr conn = it->second;
        it->second.reset();
        conn->getLoop()->runInLoop(boost::bind(&TcpConnection::connectionDestroyed, conn));
        conn.reset();
    }
}

void TcpServer::setThreadNum(int threadsNum)
{
    assert(0 <= threadsNum);
    m_threadPool->setThreadNum(threadsNum);
}

void TcpServer::start()
{
    if(!m_started)
    {
        m_started = true;
        m_threadPool->start(m_threadInitCB);
    }

    if(!m_acceptor->listenning())
    {
        m_ploop->runInLoop(boost::bind(&Acceptor::listen, get_pointer(m_acceptor)));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    m_ploop->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", m_hostPort.c_str(), m_nextConnId);
    ++m_nextConnId;
    std::string connName = m_name + buf;
    InetAddress localAddr(SocketOps::getLocalAddr(sockfd));

    EventLoop* ioLoop = m_threadPool->getNextLoop();
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    conn->setConnectionCallback(m_connectionCB);
    conn->setMessageCallback(m_messageCB);
    conn->setWriteCompleteCallback(m_writeCompleteCB);
    conn->setCloseCallback(boost::bind(&TcpServer::removeConnection, this, _1));

    ioLoop->runInLoop(boost::bind(&TcpConnection::connectionEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    m_ploop->runInLoop(boost::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    m_ploop->assertInLoopThread();
    size_t n = m_connections.erase(conn->getName());
    (void)n;
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(boost::bind(&TcpConnection::connectionDestroyed, conn));
}

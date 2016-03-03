#include "TcpConnection.h"
#include "Handler.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketOps.h"

#include <boost/bind.hpp>
#include <stdio.h>
#include <errno.h>


void defaultConnectionCB(const TcpConnectionPtr& conn)
{
    printf("%s->%s \n", conn->getLocalAddress().toIpPort().c_str(), conn->getPeerAddress().toIpPort().c_str());
}

void defaultMessageCB(const TcpConnectionPtr& conn, Buffer* buffer)
{
    buffer->retrieveAll();
}
TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr)
    : m_ploop(loop),
      m_name(name),
      m_state(kConnecting),
      m_socket(new Socket(sockfd)),
      m_handler(new Handler(loop, sockfd)),
      m_localAddr(localAddr),
      m_peerAddr(peerAddr),
      m_highWaterMark(64 * 1024 * 1024)
{
    //m_handler->setReadCallback(boost::bind(&TcpConnection::handleRead, this, _1));
    m_handler->setReadCallback(boost::bind(&TcpConnection::handleRead, this));
    m_handler->setWriteCallback(boost::bind(&TcpConnection::handleWrite, this));
    m_handler->setCloseCallback(boost::bind(&TcpConnection::handleClose, this));
    m_handler->setErrorCallback(boost::bind(&TcpConnection::handleError, this));
    printf("TcpConnection ctor:%s, fd:%d", m_name.c_str(), sockfd);
    m_socket->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    printf("TcpConnection dtor:%s, fd:%d", m_name.c_str(), m_handler->getFd());
}

void TcpConnection::send(const void* data, size_t len)
{
    if(m_state == kConnected)
    {
        if(m_ploop->isInLoopThread())
        {
            sendInLoop(data, len);
        }
        else
        {
            std::string message(static_cast<const char*>(data), len);
            m_ploop->runInLoop(boost::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

void TcpConnection::send(const std::string& message)
{
    if(m_state == kConnected)
    {
        if(m_ploop->isInLoopThread())
        {
            sendInLoop(message);
        }
        else
        {
            m_ploop->runInLoop(boost::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

void TcpConnection::send(Buffer* message)
{
    if(m_state == kConnected)
    {
        if(m_ploop->isInLoopThread())
        {
            sendInLoop(message->peek(), message->readableBytes());
        }
        else
        {
            m_ploop->runInLoop(boost::bind(&TcpConnection::sendInLoop, this, message->retrieveAllAsString()));
        }
    }

}

void TcpConnection::sendInLoop(const std::string& message)
{
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
    m_ploop->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    if(!m_handler->isWriting() && m_outputBuffer.readableBytes() == 0)
    {
        nwrote = SocketOps::write(m_handler->getFd(), data, len);
        if(nwrote >= 0)
        {
            remaining = len - nwrote;
            if(remaining == 0 && m_writeCompleteCB)
            {
                m_ploop->queueInLoop(boost::bind(m_writeCompleteCB, shared_from_this()));
            }
        }
        else
        {
            nwrote = 0;
            if(errno != EWOULDBLOCK)
            {
                printf("TcpConnection::sendInLoop error\n");
            }
        }
    }

    assert(remaining < len);
    if(remaining > 0)
    {
        printf("TcpConnection::sendInLoop going to write more data\n");
        m_outputBuffer.readableBytes();
        //size_t oldLen = m_outputBuffer.readableBytes();
        //TODO highwater
        m_outputBuffer.append(static_cast<const char*>(data) + nwrote, remaining);
        if(!m_handler->isWriting())
        {
            m_handler->enableWriting();
        }
    }

}

void TcpConnection::shutdown()
{
    if(m_state == kConnected)
    {
        setState(kDisconnecting);
        m_ploop->runInLoop(boost::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    m_ploop->assertInLoopThread();
    if(!m_handler->isWriting())
    {
        m_socket->shutdownWrite();
    }
}
void TcpConnection::setTcpNoDelay(bool on)
{
    m_socket->setTcpNoDelay(on);
}

void TcpConnection::connectionEstablished()
{
    m_ploop->assertInLoopThread();
    assert(m_state == kConnecting);
    setState(kConnected);
    m_handler->setTie(shared_from_this());
    m_handler->enableReading();
    m_connectionCB(shared_from_this());
}

void TcpConnection::connectionDestroyed()
{
    m_ploop->assertInLoopThread();
    if(m_state == kConnected);
    {
        setState(kDisconnected);
        m_handler->disableAll();
        m_connectionCB(shared_from_this());
    }
    m_handler->removeHandler();
}

void TcpConnection::handleRead(/* TODO TimeStamp receiveTime */)
{
    m_ploop->assertInLoopThread();
    int savedErrno = 0;
    ssize_t n = m_inputBuffer.readFd(m_handler->getFd(), &savedErrno);
    if(n > 0)
    {
        m_messageCB(shared_from_this(), &m_inputBuffer/*receiveTime*/);
    }
    else if(n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        printf("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    m_ploop->assertInLoopThread();
    if(m_handler->isWriting())
    {
        ssize_t n = SocketOps::write(m_handler->getFd(), m_outputBuffer.peek(), m_outputBuffer.readableBytes());
        if(n > 0)
        {
            m_outputBuffer.retrieve(n);
            if(m_outputBuffer.readableBytes() == 0)
            {
                m_handler->disableWriting();
                if(m_writeCompleteCB)
                {
                    m_ploop->queueInLoop(boost::bind(m_writeCompleteCB, shared_from_this()));
                }

                if(m_state == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
            else
            {
                printf("Going to write more data\n");
            }
        }
        else
        {
            printf("TcpConnection::handleWrite\n");
        }
    }
    else
    {
        printf("TcpConnection::handleWrite Connection is down, no more writing\n");
    }
}
void TcpConnection::handleClose()
{
    m_ploop->assertInLoopThread();
    printf("TcpConnection::handleClose state = %d\n", m_state);
    assert(m_state == kConnected || m_state == kDisconnecting);
    setState(kDisconnected);
    m_handler->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    m_connectionCB(guardThis);

    m_closeCB(guardThis);//TcpServer::removeConnection()
}
void TcpConnection::handleError()
{
    int err = SocketOps::getSocketError(m_handler->getFd());
    printf("TcpConnection::handleError %s, err %d\n", m_name.c_str(), err);

}

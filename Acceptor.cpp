#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketOps.h"
#include <boost/bind.hpp>
#include <errno.h>
#include <fcntl.h>


Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
    : m_ploop(loop),
      m_acceptSocket(SocketOps::CreateNonblockingOrDie()),
      m_acceptHandler(loop, m_acceptSocket.getSockfd()),
      m_blistenning(false),
      m_idleFd(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
   assert(m_idleFd >= 0);
   m_acceptSocket.setReuseAddr(true);
   m_acceptSocket.bindAddress(listenAddr);
   m_acceptHandler.setReadCallback(boost::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    m_acceptHandler.disableAll();
    m_acceptHandler.removeHandler();
    ::close(m_idleFd);
}

void Acceptor::listen()
{
    m_ploop->assertInLoopThread();
    m_blistenning = true;
    m_acceptSocket.listen();
    m_acceptHandler.enableReading();
}

void Acceptor::handleRead()
{
    m_ploop->assertInLoopThread();
    InetAddress peerAddr(0);
    int connfd = m_acceptSocket.accept(&peerAddr);
    if(connfd >= 0)
    {
        if(m_newConnectionCb)
        {
            m_newConnectionCb(connfd, peerAddr);
        }
        else
        {
            SocketOps::close(connfd);
        }
    }
    else
    {
        if(errno == EMFILE)
        {
            ::close(m_idleFd);
            m_idleFd = ::accept(m_acceptSocket.getSockfd(), NULL, NULL);
            ::close(m_idleFd);
            m_idleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

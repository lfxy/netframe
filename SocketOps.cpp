#include "SocketOps.h"

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>


namespace
{
    typedef struct sockaddr SA;
    
    const SA* sockaddr_cast(const struct sockaddr_in* addr)
    {
        return static_cast<const SA*>((const void*)addr);
    }

    SA* sockaddr_cast(struct sockaddr_in* addr)
    {
        return static_cast<SA*>((void*)addr);
    //    return static_cast<SA*>(implicit_cast<void*>(addr));
    }

    void setNonBlockAndCloseOnExec(int sockfd)
    {
        int flags = ::fcntl(sockfd, F_GETFL, 0);
        flags |= O_NONBLOCK;
        int ret = :: fcntl(sockfd, F_SETFL, flags);

        flags = ::fcntl(sockfd, F_GETFD, 0);
        flags |= FD_CLOEXEC;
        ret = ::fcntl(sockfd, F_SETFD, flags);

        (void)ret;
    }
}

int SocketOps::CreateNonblockingOrDie()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0)
    {
        printf("CreateNonblockingOrDie error\n");
    }
    return sockfd;
}

void SocketOps::bindOrDie(int sockfd, const struct sockaddr_in& addr)
{
    int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof(addr));
    if(ret < 0)
    {
        printf("SocketOps::bindOrDie error\n");
    }
}

int SocketOps::accept(int sockfd, struct sockaddr_in* addr)
{
    socklen_t addrlen = sizeof(*addr);
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd < 0)
    {
        int savedErrno = errno;
        printf("SocketOps::accept error\n");
        switch(savedErrno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                printf("unexpected error of ::accept %d\n", savedErrno);
                break;
            default:
                printf("unknown error of ::accept %d\n", savedErrno);
                break;
        }
    }
    return connfd;
}

int SocketOps::connect(int sockfd, const struct sockaddr_in& addr)
{
    return ::connect(sockfd, sockaddr_cast(&addr), sizeof(addr));
}

ssize_t SocketOps::read(int sockfd, void* buf, size_t count)
{
    return ::read(sockfd, buf, count);
}

ssize_t SocketOps::readv(int sockfd, const struct iovec* iov, int iovcnt)
{
    return ::readv(sockfd, iov, iovcnt);
}

ssize_t SocketOps::write(int sockfd, const void* buf, size_t count)
{
    return ::write(sockfd, buf, count);
}

void SocketOps::close(int sockfd)
{
    if(::close(sockfd) < 0)
    {
        printf("SocketOps::close\n");
    }
}

void SocketOps::shutdownWrite(int sockfd)
{
    if(::shutdown(sockfd, SHUT_WR) < 0)
    {
        printf("SocketOps::shutdownWrite\n");
    }
}

void SocketOps::toIpPort(char* buf, size_t size, const struct sockaddr_in& addr)
{
    char host[INET_ADDRSTRLEN] = "INVALID";
    //toIp(host, sizeof(host), addr);
    ::inet_ntop(AF_INET, &addr.sin_addr, host, static_cast<socklen_t>(sizeof(host)));
    uint16_t port = be16toh(addr.sin_port);
    snprintf(buf, size, "%s:%u", host, port);
}

void SocketOps::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = htobe16(port);
    if(::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        printf("SocketOps::fromIpPort\n");
    }
}

int SocketOps::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = sizeof(optval);
    if(::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

struct sockaddr_in SocketOps::getLocalAddr(int sockfd)
{
    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof(localaddr));
    socklen_t addrlen = sizeof(localaddr);
    if(::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
    {
        printf("SocketOps::getLocalAddr\n");
    }
    return localaddr;
}

struct sockaddr_in SocketOps::getPeerAddr(int sockfd)
{
  struct sockaddr_in peeraddr;
  bzero(&peeraddr, sizeof peeraddr);
  socklen_t addrlen = sizeof(peeraddr);
  if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
  {
    printf("SocketOps::getPeerAddr\n");
  }
  return peeraddr;
}

bool SocketOps::isSelfConnect(int sockfd)
{
  struct sockaddr_in localaddr = getLocalAddr(sockfd);
  struct sockaddr_in peeraddr = getPeerAddr(sockfd);
  return localaddr.sin_port == peeraddr.sin_port
      && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}

void SocketOps::toIp(char* buf, size_t size, const struct sockaddr_in& addr)
{
    assert(size >= INET_ADDRSTRLEN);
    ::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
}

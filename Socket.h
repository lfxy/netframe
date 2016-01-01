#ifndef _NET_SOCKET_H_
#define _NET_SOCKET_H_
#include <boost/noncopyable.hpp>

class InetAddress;
class Socket : boost::noncopyable
{
    public:
        explicit Socket(int sockfd)
            : m_sockfd(sockfd)
        {}
        ~Socket();
        int getSockfd(){return m_sockfd;}
        void bindAddress(const InetAddress& localaddr);
        void listen();
        int accept(InetAddress* peeraddr);
        void shutdownWrite();
        void setTcpNoDelay(bool on);
        void setReuseAddr(bool on);
        void setKeepAlive(bool on);

    private:
        const int m_sockfd;

};

#endif

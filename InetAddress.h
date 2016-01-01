#ifndef _NET_INETADDRESS_H_
#define _NET_INETADDRESS_H_

#include <netinet/in.h>
#include <string>

class InetAddress
{
    public:
        explicit InetAddress(uint16_t port);
        InetAddress(const std::string& ip, uint16_t port);
        InetAddress(const struct sockaddr_in& addr)
            : m_addr(addr){}

        std::string toIp() const;
        std::string toIpPort() const;
        std::string toHostPort() const {return toIpPort();}

        const struct sockaddr_in& getSockAddrInet() const {return m_addr;}
        void setSockAddrInet(const struct sockaddr_in& addr) {m_addr = addr;}
        uint32_t ipNetEndian() const { return m_addr.sin_addr.s_addr; }
        uint16_t portNetEndian() const { return m_addr.sin_port; }

    private:
        struct sockaddr_in m_addr;
};


#endif

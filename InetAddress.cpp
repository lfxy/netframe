#include "InetAddress.h"
#include "SocketOps.h"
#include <strings.h>
#include <netinet/in.h>
#include <stdint.h>
#include <boost/static_assert.hpp>

static const in_addr_t kInaddrAny = INADDR_ANY;
BOOST_STATIC_ASSERT(sizeof(InetAddress) == sizeof(struct sockaddr_in));

InetAddress::InetAddress(uint16_t port)
{
    bzero(&m_addr, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = SocketOps::hostToNetwork32(kInaddrAny);
    m_addr.sin_port = SocketOps::hostToNetwork16(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
    bzero(&m_addr, sizeof(m_addr));
    SocketOps::fromIpPort(ip.data(), port, &m_addr);
}

std::string InetAddress::toIp() const
{
    char buf[32];
    SocketOps::toIp(buf, sizeof(buf), m_addr);
    return buf;
}

std::string InetAddress::toIpPort() const
{
    char buf[32];
    SocketOps::toIpPort(buf, sizeof(buf), m_addr);
    return buf;
}

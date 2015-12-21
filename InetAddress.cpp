#include "InetAddress.h"
#include <netinet/in.h>
#include <stdint.h>
#include <string.h>

static const in_addr_t kInaddrAny = INADDR_ANY;

InetAddress::InetAddress(uint16_t port)
{
    bzero(&m_addr, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = htobe32(kInaddrAny);
    m_addr.sin_port = htobe16(port);
}

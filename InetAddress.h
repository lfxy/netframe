#ifndef _NET_INETADDRESS_H_
#define _NET_INETADDRESS_H_

#include <netinet/in.h>

class InetAddress
{
    public:
        explicit InetAddress(uint16_t port);
        InetAddress(const struct sockaddr_in& addr)
            : m_addr(addr){}


    private:
        struct sockaddr_in m_addr;
};


#endif

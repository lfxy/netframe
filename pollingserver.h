#ifndef _POLLING_SERVER_H_
#define _POLLING_SERVER_H_
#include <string>
#include <map>
#include <sys/epoll.h>


class PollingServer
{
public:
    PollingServer();
    ~PollingServer();
    void Init(std::string& name);
    void Run();
    void Release();
    void AddFd(int epollfd, int fd, bool enable_et);
    void LtModel(epoll_event* events, int number, int epollfd, int listenfd);

private:
    int m_setNonblocking(int fd);

private:
    std::string m_ServicName;
    std::map<std::string, int> m_serviceKey;
    std::string m_key;

};
#endif

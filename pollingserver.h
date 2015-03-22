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
    void Init(std::string& name, std::string ip, int port);
    void Run();
    void Release();
    //void AddFd(int epollfd, int fd, bool enable_et);
    void AddFd(int epollfd, int fd, bool enableread, bool enablewrite, bool bfirst, bool enable_et);
    void LtModel(epoll_event* events, int number, int epollfd, int listenfd);
    void EtModel(epoll_event* events, int number, int epollfd, int listenfd);

private:
    int m_setNonblocking(int fd);
    std::string m_convertToStr(int value);

private:
    std::string m_servicName;
    std::map<std::string, int> m_serviceKey;
    std::string m_key;
    bool m_running;
    std::string m_serviceIp;
    int m_servicePort;

};
#endif

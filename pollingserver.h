#ifndef _POLLING_SERVER_H_
#define _POLLING_SERVER_H_
#include <string>
#include <map>
#include <sys/epoll.h>
#include <sstream>
//#include <fstream>
#include <stdio.h>

#include "timestamp.h"

class PollingServer
{
public:
    PollingServer();
    ~PollingServer();
    void Init(std::string& name, std::string ip, int port);
    void Run();
    void Release();
    //void AddFd(int epollfd, int fd, bool enable_et);
    //void AddFd(int epollfd, int fd, bool enableread, bool enablewrite, bool bfirst, bool enable_et);
    void AddFd(int epollfd, int fd, int ev);
    void ModFd(int epollfd, int fd, int ev);
    void LtModel(epoll_event* events, int number, int epollfd, int listenfd);
    //void EtModel(epoll_event* events, int number, int epollfd, int listenfd);

private:
    int m_setNonblocking(int fd);
    char* m_getCurrentTime(std::string& id);
    void m_handleListenFd(int listenfd, int epollfd);
    int m_handleReadfd(int sockfd, int epollfd);
    void m_handleSendFd(int sockfd, int epollfd);

private:
    class ConvertToString
    {
    public:
        template<class T>
        std::string& GetString(T& src)
        {
            m_strStream.str("");
            m_strStream << src;
            m_strDec = m_strStream.str();
            return m_strDec;
        }

    private:
        std::stringstream m_strStream;
        std::string m_strDec;
    };

    std::string m_servicName;
    std::map<std::string, int> m_serviceKey;
    std::string m_key;
    bool m_running;
    std::string m_serviceIp;
    int m_servicePort;
    timeStamp* m_readTime;
    timeStamp* m_convertTime;
    timeStamp* m_sendTime;
    timeStamp* m_fileTime;
    int m_timeCheck;
//    std::ofstream m_outTimeFile;
    //FILE* m_fileFd;
    std::string m_strTime;
    bool m_hasSendData;
    char* m_logBuf;

    ConvertToString m_convertStr;
};

#endif

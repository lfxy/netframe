#include<iostream>
#include <sys/time.h>
#include <unistd.h>
#include <string>


class timeStamp
{
public:

    timeStamp(std::string name)
    {
        m_tt.tv_sec = 0;
        m_tt.tv_usec = 0;
        m_start = m_end = 0;
        m_interval = 0;
        m_name = name;
    }
    ~timeStamp(){}
    void StartRecord()
    {
        gettimeofday(&m_tt, NULL);
        m_start = m_tt.tv_sec * 1000000 + m_tt.tv_usec;
    }
    void EndRecord()
    {
        ++m_time;
        gettimeofday(&m_tt, NULL);
        m_end = m_tt.tv_sec * 1000000 + m_tt.tv_usec;
        m_interval += m_end - m_start;
        if(m_time >= 30000)
        {
            m_time = 0;
            printf("%s 30000 times used : %ld\n", m_name.c_str(), m_interval);
        }
    }

    long GetTime()
    {
        return m_interval;
    }
private:
    int m_time;
    std::string m_name;
    long m_start;
    long m_end;
    struct timeval m_tt;
    long m_interval;


};

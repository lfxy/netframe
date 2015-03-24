#include<iostream>
#include <sys/time.h>
#include <unistd.h>


class timeStamp
{
public:

    timeStamp()
    {
        m_tt.tv_sec = 0;
        m_tt.tv_usec = 0;
        m_start = m_end = 0;
    }
    ~timeStamp(){}
    void StartRecord()
    {
        gettimeofday(&m_tt, NULL);
        m_start = m_tt.tv_sec * 1000000 + m_tt.tv_usec;
    }
    void EndRecord()
    {
        gettimeofday(&m_tt, NULL);
        m_end = m_tt.tv_sec * 1000000 + m_tt.tv_usec;
    }

    long GetTime()
    {
        return m_end - m_start;
    }
private:
    long m_start;
    long m_end;
    struct timeval m_tt;


};

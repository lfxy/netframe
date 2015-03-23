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
    void StartEnd()
    {
        gettimeofday(&m_tt, NULL);
        m_end = m_tt.tv_sec * 1000000 + m_tt.tv_usec;
    }

    int GetTime()
    {
        return m_end - m_start;
    }
private:
    int m_start;
    int m_end;
    struct timeval m_tt;


};

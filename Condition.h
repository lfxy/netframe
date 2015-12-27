#ifndef _CONDITION_H_
#define _CONDITION_H_

#include "Mutex.h"
#include <boost/noncopyable.hpp>
#include <pthread.h>

class Condition : boost::noncopyable
{
    public:
        explicit Condition(MutexLock& mutex)
            : m_mutex(mutex)
        {
            pthread_cond_init(&m_cond, NULL);
        }
        ~Condition()
        {
            pthread_cond_destroy(&m_cond);
        }

        void wait()
        {
            pthread_cond_wait(&m_cond, m_mutex.getPthreadMutex());
        }
        bool waitForSeconds(int seconds);
        void notify()
        {
            pthread_cond_signal(&m_cond);
        }
        void notifyAll()
        {
            pthread_cond_broadcast(&m_cond);
        }
    private:
        MutexLock& m_mutex;
        pthread_cond_t m_cond;
};

#endif


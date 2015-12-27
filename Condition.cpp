#include "Condition.h"
#include <errno.h>

bool Condition::waitForSeconds(int seconds)
{
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    abstime.tv_sec += seconds;
    return ETIMEDOUT == pthread_cond_timedwait(&m_cond, m_mutex.getPthreadMutex(), &abstime);
}

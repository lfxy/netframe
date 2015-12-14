#ifndef _CURRENTTHREAD_H_
#define _CURRENTTHREAD_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>


namespace CurrentThread
{
    extern __thread int t_cachedTid;
    extern char ctest[10];

    void cacheTidLocal();


    inline int tid()
    {
        if(t_cachedTid == 0)
        {
            cacheTidLocal();
        }
        return t_cachedTid;
    }
    inline void testns(){
        printf("aaa, %s\n", ctest);
    }

}

#endif

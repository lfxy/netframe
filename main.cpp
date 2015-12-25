/*************************************************************************
    > File Name: main.cpp
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Sun 08 Mar 2015 01:38:31 AM PST
 ************************************************************************/

#include "pollingserver.h"
#include<iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "EventLoop.h"
#include "Handler.h"
#include <sys/timerfd.h>

using namespace std;

#define MAX_EVENT_NUMBER 1024

EventLoop* g_loop;

void timeout()
{
    printf("Timout!\n");
    g_loop->quit();
}


int main(int argc, char** argv)
{
/*    if(argc < 3)
    {
        printf("Need ip and port\n");
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);*/
    /*const char* ip = "127.0.0.1";
    int port = 8086;

    PollingServer pollserv;
    std::string servname = "GenerateId";
    pollserv.Init(servname, ip, port);
    pollserv.Run();
    pollserv.Release();*/

    EventLoop loop;
    g_loop = &loop;
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Handler handler(&loop, timerfd);
    handler.setReadCallback(timeout);
    handler.enableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);

    loop.loop();

    ::close(timerfd);

    return 0;
}

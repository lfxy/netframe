/*************************************************************************
    > File Name: main.cpp
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Sun 08 Mar 2015 01:38:31 AM PST
 ************************************************************************/

#include "pollingserver.h"
#include<iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

using namespace std;

#define MAX_EVENT_NUMBER 1024


int main(int argc, char** argv)
{
    if(argc < 3)
    {
        printf("Need ip and port\n");
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    ret = bind(listenfd, (struct sockaddr*) &address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    PollingServer pollserv;
    pollserv.AddFd(epollfd, listenfd, true);
    while(1)
    {
        int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        printf("After epoll_wait here\n");
        if(ret < 0)
        {
            printf("epoll failure\n");
            break;
        }
    pollserv.LtModel(events, ret, epollfd, listenfd);
    }
    close(listenfd);
    return 0;
}

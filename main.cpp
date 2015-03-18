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

    PollingServer pollserv;
    std::string servname = "GenerateId";
    pollserv.Init(servname, ip, port);
    pollserv.Run();
    pollserv.Release();
    return 0;
}

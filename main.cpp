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
#include "EventLoopThread.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "SocketOps.h"
#include "TcpServer.h"

using namespace std;

#define MAX_EVENT_NUMBER 1024

EventLoop* g_loop;

void timeout()
{
    printf("Timout!\n");
    g_loop->quit();
}

void test1()
{
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
}

void runInThread()
{
    printf("runInThread():pid=%d, tid=%d\n", getpid(), CurrentThread::tid());
}

void test_EventLoopThread()
{
    printf("test2():pid=%d, tid=%d\n", getpid(), CurrentThread::tid());

    EventLoopThread loopThread;
    EventLoop* loop = loopThread.startLoop();
    printf("main 1111111111\n");
    loop->runInLoop(runInThread);
    printf("main 222222222222\n");
    sleep(1);
    loop->quit();
    printf("exit test2\n");
}

void newConnection(int sockfd, const InetAddress& peerAddr)
{
    printf("newConnection:addcept a new connection from %s\n", peerAddr.toHostPort().c_str());
    ::write(sockfd, "how are you?\n", 13);
    SocketOps::close(sockfd);
}
void test_Acceptor()
{
    printf("test_Acceptor:pid = %d\n", getpid());
    InetAddress listenaddr(9981);
    EventLoop loop;
    Acceptor acceptor(&loop, listenaddr);
    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();
    loop.loop();
}

std::string message;

void onConnection(const TcpConnectionPtr& conn)
{
    if(conn->isConnected())
    {
        printf("onConnection: new connection [%s] from %s\n", conn->getName().c_str(), conn->getPeerAddress().toHostPort().c_str());
        conn->send(message);
    }
    else
    {
        printf("onConnection: connection [%s] is down\n", conn->getName().c_str());
    }
}

void onWriteComplete(const TcpConnectionPtr& conn)
{
    conn->send(message);
}

void onMessage(const TcpConnectionPtr& conn,
               Buffer* buf)
{
  printf("onMessage(): received %d bytes from connection [%s]\n",
         buf->readableBytes(),
         conn->getName().c_str());

    printf("ccccccccc:%s\n", buf->retrieveAllAsString().c_str());
  buf->retrieveAll();
}


void test_TcpServer()
{
    std::string servername = "test_Tcpserver";
    message = "bbbbbbbbbbbbbbb\n";
    InetAddress listenAddr(9981);
    EventLoop loop;
    TcpServer server(&loop, listenAddr, servername);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.setWriteCompleteCallback(onWriteComplete);
    server.start();

    loop.loop();
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
    //test_EventLoopThread();
    //test_Acceptor();
    test_TcpServer();

    return 0;
}

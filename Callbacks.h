#ifndef _NET_CALLBACKS_H_
#define _NET_CALLBACKS_H_

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
//#include "Timestamp.h"

class Buffer;
class TcpConnection;

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef boost::function<void()> TimerCallback;
typedef boost::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef boost::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef boost::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef boost::function<void (const TcpConnectionPtr&)> HighWaterCallback;
typedef boost::function<void (const TcpConnectionPtr&, Buffer* /*, Timestamp*/)> MessageCallback;

void defaultConnectionCB(const TcpConnectionPtr& conn);
void defaultMessageCB(const TcpConnectionPtr& conn, Buffer* buffer);

#endif

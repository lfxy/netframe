// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <Thread.h>
#include <CurrentThread.h>
#include <Exception.h>
//#include <muduo/base/Logging.h>

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>



namespace CurrentThread
{
    __thread int t_cachedTid = 0;
    char ctest[10];
}


namespace detail
{
    pid_t gettid()
    {
      return static_cast<pid_t>(::syscall(SYS_gettid));
    }
}

void CurrentThread::cacheTidLocal()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = detail::gettid();
    //int n = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
    //assert(n == 6); (void) n;
  }
}

Thread::Thread(const ThreadFunc& func, const std::string& n)
  : started_(false),
    pthreadId_(0),
    tid_(0),
    func_(func),
    name_(n)
{
 // numCreated_.increment();
}

Thread::~Thread()
{
  // no join
}

void Thread::start()
{
  assert(!started_);
  started_ = true;
  errno = pthread_create(&pthreadId_, NULL, &startThread, this);
  if (errno != 0)
  {
      printf("Thread::start pthread_create error\n");
    //LOG_SYSFATAL << "Failed in pthread_create";
  }
}

int Thread::join()
{
  assert(started_);
  return pthread_join(pthreadId_, NULL);
}

void* Thread::startThread(void* obj)
{
  Thread* thread = static_cast<Thread*>(obj);
  thread->runInThread();
  return NULL;
}

void Thread::runInThread()
{
  tid_ = CurrentThread::tid();
  func_();
 /* CurrentThread::t_threadName = name_.c_str();
  try
  {
    func_();
    CurrentThread::t_threadName = "finished";
  }
  catch (const Exception& ex)
  {
    CurrentThread::t_threadName = "crashed";
    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
    abort();
  }
  catch (const std::exception& ex)
  {
    CurrentThread::t_threadName = "crashed";
    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  }
  catch (...)
  {
    CurrentThread::t_threadName = "crashed";
    fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
    throw; // rethrow
  }
  */
}


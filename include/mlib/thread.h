/*!
  \file thread.h thread class definition.

  (c) Mircea Neacsu 1999-2017
*/
#pragma once

#include "event.h"
#include <functional>

namespace mlib {

/// Wrapper for a Windows %thread
class thread : public syncbase
{
public:
  thread (std::function<int ()> func);
  virtual ~thread   ();

  virtual void start ();
  void fork();
  void join ();

  DWORD id () const;
  UINT result () const;
  bool is_running () const;
  int priority  ();
  void priority  (int pri);
  virtual void resume    ();
  virtual void suspend   ();

protected:
  thread (const char *name=0, bool inherit=false, DWORD stack_size=0, PSECURITY_DESCRIPTOR sd=NULL);

  virtual bool init ();
  virtual void term ();
  virtual void run ();

  UINT exitcode;                                 ///< exit code

private:
  thread (HANDLE handle, DWORD id);               ///< ctor used by current %thread.
  void initialize ();
  thread& operator= (const thread& t) = delete;
  thread (const thread& t) = delete;

  DWORD id_;
  bool volatile running;
  bool shouldKill;
  event created, started;
  static UINT _stdcall entryProc( thread *ts );
  friend class current_thread;
  SECURITY_ATTRIBUTES sa;
  DWORD stack;
  std::function<int ()> thfunc;
};

/// Currently executing %thread object
class current_thread : protected thread
{
public:
  current_thread();
  ~current_thread ();
  thread::id;
  thread::handle;
  thread::is_running;
  thread::priority;

protected:
  void run() {};
};


//inlines

/// Another name for start () function
inline
void thread::fork ()
{
  start ();
}

/// Another name for wait () function
inline
void thread::join ()
{
  wait (INFINITE);
}

/// Return thread's ID
inline
DWORD thread::id () const 
{
  return id_;
};

/// Return thread's exit code 
inline
UINT thread::result () const 
{
  return exitcode;
}

/// Return _true_ if %thread is running
inline
bool thread::is_running () const
{
  return running;
}

/// Return priority of a thread
inline
int thread::priority()
{
  return GetThreadPriority (handle ());
}

/// Set priority of a thread
inline
void thread::priority (int pri)
{
  SetThreadPriority (handle (), pri);
}

/// Initialization function called before run
inline
bool thread::init ()
{
  return true;
}

/// Finalization function called after run
inline
void  thread::term ()
{
  running = false;
};


}

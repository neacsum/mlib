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
  /// Thread state
  enum class state {ready, starting, running, ending, finished};
  thread (std::function<unsigned int ()> func);
  virtual ~thread ();

  virtual void start ();
  void fork();
  void join ();
  DWORD wait (DWORD time_limit = INFINITE);
  DWORD wait_alertable (DWORD time_limit = INFINITE);
  DWORD wait_msg (DWORD time_limit = INFINITE, DWORD mask = QS_ALLINPUT);


  DWORD id () const;
  UINT result () const;
  bool is_running () const;
  state get_state () const;
  int priority ();
  void priority (int pri);
  using syncbase::name;
  virtual void name (const std::string& nam);

protected:
  thread (const std::string& name = std::string (), DWORD stack_size = 0, 
    PSECURITY_DESCRIPTOR sd = NULL, bool inherit = false);

  virtual bool init ();
  virtual void term ();
  virtual void run ();

  unsigned int exitcode;                          ///< exit code

private:
  void initialize (PSECURITY_DESCRIPTOR sd, BOOL inherit);
  thread& operator= (const thread& t) = delete;
  thread (const thread& t) = delete;

  DWORD id_;
  state volatile stat;
  event created, started;
  static unsigned int _stdcall entryProc (thread *ts);
  DWORD stack;
  std::function<unsigned int ()> thfunc;
  std::exception_ptr pex;
};

/*!
  Currently executing %thread object
  \ingroup syncro

  Useful for accessing properties like id, handle, priority etc.
*/
class current_thread
{
public:
  DWORD id () const;
  HANDLE handle() const;
  int priority ();
  void priority (int pri);
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
  return stat == state::running;
}

/// Return thread status
inline
thread::state thread::get_state () const
{
  return stat;
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
}

// ----------- current_thread inline functions ------------------------------

/// Return ID of current thread
inline
DWORD current_thread::id () const
{
  return GetCurrentThreadId ();
}

inline
HANDLE current_thread::handle () const
{
  return GetCurrentThread ();
}

/// Return priority current thread
inline
int current_thread::priority ()
{
  return GetThreadPriority (GetCurrentThread ());
}

/// Set priority of current thread
inline
void current_thread::priority (int pri)
{
  SetThreadPriority (GetCurrentThread (), pri);
}

}

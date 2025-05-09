/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file thread.h Definition of mlib::thread class

#pragma once

#include "event.h"
#include <functional>

namespace mlib {

/// Wrapper for a Windows %thread
class thread : public syncbase
{
public:

  /// Execution state of a thread
  enum class state
  {
    ready,        ///< not started
    starting,     ///< in the process of starting up
    running,      ///< is running
    ending,       ///< in the process of finishing
    finished      ///< execution finished
  };

  /// Make a thread with the given function body
  thread (std::function<unsigned int ()> func);

  virtual ~thread ();

  /// Begin execution of a newly created thread
  virtual void start ();

  /// Another name for start () function
  void fork ();

  /// Another name for wait () function
  void join ();

  /// Wait for thread to finish execution
  DWORD wait (DWORD time_limit = INFINITE);

  /// Wait for thread to finish or an APC, or IO completion routine to occur
  DWORD wait_alertable (DWORD time_limit = INFINITE);

  /// Wait for thread to finish or a message to be queued
  DWORD wait_msg (DWORD time_limit = INFINITE, DWORD mask = QS_ALLINPUT);

  /// Rethrow an exception usually in the context of another thread
  void rethrow_exception () const;

  /// Return thread's ID
  DWORD id () const;

  /// Return thread's exit code
  UINT result () const;

  /// Return _true_ if %thread is running
  bool is_running () const;

  /// Return thread's execution status
  state get_state () const;

  /// Return thread's priority
  int priority () const;

  /// Set thread's priority
  void priority (int pri);

  using syncbase::name;

  /// Set thread's name
  virtual void name (const std::string& nam);

protected:
  /// Protected constructor for use of thread-derived objects
  thread (const std::string& name = std::string (), DWORD stack_size = 0,
          PSECURITY_DESCRIPTOR sd = NULL, bool inherit = false);

  /// Initialization function called before run
  virtual bool init ();

  /// Finalization function called after run
  virtual void term ();

  /// Default run function
  virtual void run ();

  unsigned int exitcode; ///< exit code

private:
  void initialize (PSECURITY_DESCRIPTOR sd, BOOL inherit);
  thread& operator= (const thread& t) = delete;
  thread (const thread& t) = delete;

  DWORD id_;
  state volatile stat;
  auto_event created, started;
  static unsigned int _stdcall entryProc (thread* ts);
  DWORD stack;
  std::function<unsigned int ()> thfunc;
  std::exception_ptr pex;
};

/*!
  Currently executing thread object
  \ingroup syncro

  Useful for accessing properties like id, handle, priority etc.
*/
class current_thread
{
public:
  DWORD id () const;      ///< Return ID of current thread
  HANDLE handle () const; ///< Return handle of current thread
  int priority ();        ///< Return priority of current thread
  void priority (int pri); ///< set priority of current thread
};

// inlines

inline void thread::fork ()
{
  start ();
}

inline void thread::join ()
{
  wait (INFINITE);
}

inline DWORD thread::id () const
{
  return id_;
};

inline UINT thread::result () const
{
  return exitcode;
}

inline bool thread::is_running () const
{
  return stat == state::running;
}

inline thread::state thread::get_state () const
{
  return stat;
}

inline int thread::priority () const
{
  return GetThreadPriority (handle ());
}

inline void thread::priority (int pri)
{
  SetThreadPriority (handle (), pri);
}

inline bool thread::init ()
{
  return true;
}

inline void thread::term ()
{
}

inline void thread::rethrow_exception () const
{
  if (pex)
    std::rethrow_exception (pex);
}

// ----------- current_thread inline functions ------------------------------

/// Return ID of current thread
inline DWORD current_thread::id () const
{
  return GetCurrentThreadId ();
}

inline HANDLE current_thread::handle () const
{
  return GetCurrentThread ();
}

/// Return priority current thread
inline int current_thread::priority ()
{
  return GetThreadPriority (GetCurrentThread ());
}

/// Set priority of current thread
inline void current_thread::priority (int pri)
{
  SetThreadPriority (GetCurrentThread (), pri);
}

/*!
  Specialization of wait functions for threads, re-throw any exceptions that
  might have occurred during thread execution. Only the first exception is
  re-thrown.

@{
*/

template <typename T>
concept ThreadDerived = std::derived_from<T, mlib::thread>;

template <ThreadDerived T>
inline DWORD wait_all (const T* objs, int count, DWORD msec = INFINITE)
{
  assert (count < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  for (int i = 0; i < count; i++)
    harr[i] = objs[i].handle ();

  DWORD result = WaitForMultipleObjects (count, harr, true, msec);
  if (result < WAIT_OBJECT_0 + count)
  {
    for (int i = 0; i < count; i++)
      objs[i].rethrow_exception ();
  }

  return result;
}

template <ThreadDerived T>
inline DWORD wait_all (std::initializer_list<const T*> objs, std::chrono::milliseconds limit)
{
  assert (objs.size () < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  int i = 0;
  for (auto& th : objs)
    harr[i++] = th->handle ();

  DWORD msec = (DWORD)limit.count ();
  DWORD result = WaitForMultipleObjects ((DWORD)objs.size (), harr, true, msec);
  if (result < WAIT_OBJECT_0 + objs.size ())
  {
    for (auto& th : objs)
      th->rethrow_exception ();
  }

  return result;
}

template <ThreadDerived T>
inline DWORD wait_all (std::initializer_list<const T*> objs, DWORD msec = INFINITE)
{
  assert (objs.size () < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  int i = 0;
  for (auto& th : objs)
    harr[i++] = th->handle ();

  DWORD result = WaitForMultipleObjects ((DWORD)objs.size (), harr, true, msec);
  if (result < WAIT_OBJECT_0 + objs.size ())
  {
    for (auto& th : objs)
      th->rethrow_exception ();
  }

  return result;
}

template <ThreadDerived T>
inline DWORD wait_any (const T* objs, int count, DWORD msec = INFINITE)
{
  assert (count < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  for (int i = 0; i < count; i++)
    harr[i] = objs[i].handle ();

  DWORD result = WaitForMultipleObjects (count, harr, false, msec);
  if (result < WAIT_OBJECT_0 + count)
  {
    for (int i = 0; i < count; i++)
      objs[i].rethrow_exception ();
  }
  return result;
}

template <ThreadDerived T>
inline DWORD wait_any (std::initializer_list<const T*> objs, DWORD msec = INFINITE)
{
  assert (objs.size () < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  int i = 0;
  for (auto& th : objs)
    harr[i++] = th->handle ();

  DWORD result = WaitForMultipleObjects ((DWORD)objs.size (), harr, false, msec);
  if (result < WAIT_OBJECT_0 + objs.size ())
  {
    for (auto& th : objs)
      th->rethrow_exception ();
  }
  return result;
}

template <ThreadDerived T>
inline DWORD wait_any (std::initializer_list<const T*> objs, std::chrono::milliseconds timeout)
{
  assert (objs.size () < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  int i = 0;
  for (auto& th : objs)
    harr[i++] = th->handle ();
  DWORD msec = (DWORD)timeout.count ();
  DWORD result = WaitForMultipleObjects ((DWORD)objs.size (), harr, false, msec);
  if (result < WAIT_OBJECT_0 + objs.size ())
  {
    for (auto& th : objs)
      th->rethrow_exception ();
  }
  return result;
}

template <ThreadDerived T>
inline DWORD wait_msg (const T* objs, int count, bool all = true, DWORD msec = INFINITE,
                       DWORD mask = QS_ALLINPUT)
{
  assert (count < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  for (int i = 0; i < count; i++)
    harr[i] = objs[i].handle ();

  DWORD result = MsgWaitForMultipleObjects (count, harr, all, msec, mask);
  if (result < WAIT_OBJECT_0 + count)
  {
    for (int i = 0; i < count; i++)
      objs[i].rethrow_exception ();
  }
  return result;
}

template <ThreadDerived T>
inline DWORD wait_msg (std::initializer_list<const T*> objs, bool all = true, DWORD msec = INFINITE,
                       DWORD mask = QS_ALLINPUT)
{
  assert (objs.size () < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  int i = 0;
  for (auto& th : objs)
    harr[i++] = th->handle ();

  DWORD result = MsgWaitForMultipleObjects ((DWORD)objs.size (), harr, all, msec, mask);
  if (result < WAIT_OBJECT_0 + objs.size ())
  {
    for (auto& th : objs)
      th->rethrow_exception ();
  }
  return result;
}

///@}

} // namespace mlib

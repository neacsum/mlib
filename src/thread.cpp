/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

#include <mlib/mlib.h>
#pragma hdrstop
#include <process.h>
#include <assert.h>
#include <utf8/utf8.h>

namespace mlib {

//----------------- thread member functions ---------------------------

/*!
  \class thread
  \ingroup syncro

  Thread objects can be created by providing a function that will be run in a
  separate execution thread or by deriving an new object that reimplements the
  thread::run() function. Either way, thread objects are created in a
  "suspended animation" state. To start them use the thread::start() function.

  When combining objects and multi-threading it is useful to define what
  member functions are \e foreign (i.e. can be called by another execution
  thread) and what functions are \e owned (i.e. can be called only by the same
  execution thread). As a general rule, "owned" functions should be made private
  or protected. The object's constructors and destructor are inherently \e foreign
  while the run() function is inherently \e owned.

  Exceptions thrown while executing a thread that are not caught by user
  handlers are caught by a `try...catch` block that encompasses the thread::run()
  function and re-thrown by the thread::wait() function (considered a _foreign_
  function).
*/

/*!
  \param name       thread name (mostly for debugging purposes)
  \param stack_size thread stack size or 0 for default size
  \param sd         pointer to a security descriptor or NULL
  \param inherit    if true, thread handle is inherited by child processes
*/
thread::thread (const std::string& name, DWORD stack_size, PSECURITY_DESCRIPTOR sd, bool inherit)
  : syncbase (name)
  , stat (state::ready)
  , exitcode (0)
  , stack (stack_size)
{
  initialize (sd, inherit);
  if (!name.empty ())
    SetThreadDescription (handle (), utf8::widen (name).c_str ());
  TRACE2 ("Created thread %s[0x%x]", name.c_str (), id ());
}

/*!
  Uses a polymorphic function wrapper that can be a lambda expression, a bind
  expression or any other type of function object as a run function of the
  newly created thread. When the thread is started (using the start() function),
  the run function is called.

  The return value of the run function becomes the exit code of the thread.
*/
thread::thread (std::function<unsigned int ()> func)
  : stat (state::ready)
  , exitcode (0)
  , stack (0)
  , thfunc (func)
{
  initialize (nullptr, false);
  TRACE2 ("Created thread [0x%x]", id ());
}

///  Does the real work of creating and starting the thread
void thread::initialize (PSECURITY_DESCRIPTOR sd, BOOL inherit)
{
  // setup SECURITY_ATTRIBUTES structure
  SECURITY_ATTRIBUTES sa{sizeof (SECURITY_ATTRIBUTES), sd, inherit};

  // create thread in suspended state
  HANDLE handle = (HANDLE)_beginthreadex (&sa, stack, (unsigned int (__stdcall*) (void*))entryProc,
                                          this, CREATE_SUSPENDED, (UINT*)&id_);
  assert (handle);
  set_handle (handle);

  // Let thread run the initialization code. This will signal the "created"
  // semaphore than wait for "run" semaphore.
  ResumeThread (handle);

  // Wait for "created" semaphore to become signaled
  created.wait ();
}

/*!
  Destructor. Normally the thread should have ended earlier ( !isRunning )
  Anyhow we will use now brute force (TerminateThread) to end it.
*/
thread::~thread ()
{
  TRACE2 ("Thread %s[0x%x] in destructor", name ().c_str (), id_);
  if (stat == state::running || stat == state::starting)
  {
    TRACE ("WARNING! thread was still running");
    TerminateThread (handle (), 0);
  }
  else if (stat == state::ready)
  {
    TRACE ("Terminating thread that was not started");
    TerminateThread (handle (), 0);
  }
  id_ = 0;
}

/*!
  Static entry procedure for all threads. Assumes the passed argument is
  a thread object pointer and calls the virtual Run
*/
UINT _stdcall thread::entryProc (thread* th)
{
  th->created.signal ();
  th->started.wait ();
  try
  {
    if (th->init ())
    {
      th->stat = state::running;
      th->run ();
    }
    th->stat = state::ending;
    th->term ();
  }
  catch (...)
  {
    th->pex = std::current_exception ();
    TRACE ("Thread %s[0x%x] exception !!", th->name ().c_str (), th->id_);
  }

  TRACE2 ("Thread %s[0x%x] is ending", th->name ().c_str (), th->id_);
  th->stat = state::finished;
  _endthreadex (th->exitcode);
  return th->exitcode;
}

/*!
  Calls user supplied function if there is one
*/
void thread::run ()
{
  if (thfunc)
    exitcode = thfunc ();
}

void thread::name (const std::string& nam)
{
#ifdef MLIB_HAS_UTF8_LIB
  SetThreadDescription (handle (), utf8::widen (nam).c_str ());
#endif
  syncbase::name (nam);
}

void thread::start ()
{
  assert (handle ());
  assert (stat == state::ready);

  TRACE2 ("Thread %s[0x%x] is starting", name ().c_str (), id_);
  stat = state::starting;
  started.signal ();
  while (stat < state::running)
    Sleep (0);
}

/*!
  \param time_limit time-out interval, in milliseconds
  \return `WAIT_OBJECT_0` thread finished
  \return `WAIT_TIMEOUT` time-out interval expired

  If thread has finished and an exception occurred during thread execution,
  it is re-thrown now.
*/
DWORD
thread::wait (DWORD time_limit)
{
  DWORD ret = syncbase::wait (time_limit);
  if (ret == WAIT_OBJECT_0 && pex)
    std::rethrow_exception (pex);
  return ret;
}

/*!
  \param time_limit time-out interval, in milliseconds
  \return `WAIT_OBJECT_0` thread finished
  \return `WAIT_TIMEOUT` time-out interval expired
  \return `WAIT_IO_COMPLETION` wait ended by one or more APC-es queued.

  If thread has finished and an exception occurred during thread execution,
  it is re-thrown now.
*/
DWORD
thread::wait_alertable (DWORD time_limit)
{
  DWORD ret = syncbase::wait_alertable (time_limit);
  if (ret == WAIT_OBJECT_0 && pex)
    std::rethrow_exception (pex);
  return ret;
}

/*!
  \param time_limit   time-out interval, in milliseconds
  \param mask         input message types
  \return `WAIT_OBJECT_0`     thread finished
  \return `WAIT_TIMEOUT`      time-out interval expired
  \return `WAIT_OBJECT_0+1`   Input message received.

  If an exception occurred during thread execution, it is re-thrown now.
*/
DWORD
thread::wait_msg (DWORD time_limit, DWORD mask)
{
  DWORD ret = syncbase::wait_msg (time_limit, mask);
  if (ret == WAIT_OBJECT_0 && pex)
    std::rethrow_exception (pex);
  return ret;
}

} // namespace mlib

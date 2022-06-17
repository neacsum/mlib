/*!
  \file thread.cpp thread class implementation.

  (c) Mircea Neacsu 1999

*/

#include <process.h>
#include <assert.h>
#include <mlib/trace.h>
#include <mlib/thread.h>


namespace mlib {

//----------------- current_thread member functions -------------------
/*!
  \class current_thread 
  \ingroup syncro

  Useful for accessing properties like id, handle, priority etc.
*/

///Constructor
current_thread::current_thread() :
  thread( GetCurrentThread(), GetCurrentThreadId() )
{
}

///Destructor
current_thread::~current_thread()
{
  TRACE2 ("Current thread destructor");
}

//----------------- thread member functions ---------------------------

/*!
  \class thread 
  \ingroup syncro

  This class is abstract and the user has to derive another object and implement
  the run() function. Thread objects are created in a "suspended animation" 
  state. To start them use the start() function.

  When combining objects and multi-threading it is useful to define what 
  member functions are \e foreign (i.e. can be called by another execution
  %thread) and what functions are \e owned (i.e. can be called only by the same
  execution %thread. If possible, "owned" functions should be made private or
  protected. The object's constructors and destructor are inherently \e foreign 
  while the run function is inherently \e owned.
*/

/*!
  Private constructor used only by current_thread class
*/
thread::thread (HANDLE h, DWORD i) :
  syncbase (NULL),
  shouldKill (false),
  stat (state::running),
  exitcode (0),
  id_ (i)
{
  set_handle (h);
}

/*!
  Constructor for another thread
*/
thread::thread (const char *name, bool inherit, DWORD stack_size, PSECURITY_DESCRIPTOR sd)
  : syncbase (name)
  , shouldKill (true)
  , started (event::manual)
  , stat (state::ready)
  , exitcode (0)
  , stack (stack_size)
{
  //setup SECURITY_ATTRIBUTES structure
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = sd;
  sa.bInheritHandle = inherit;
  initialize ();
  TRACE2 ("Created thread %s[%x]", name?name:"", id());
}

/*!
  Make a thread from a function. 
  
  Uses a polymorphic function wrapper that can be a lambda expression, a bind
  expression or any other type of function object as a run function of the 
  newly created thread. When the thread is started (using the start() function), 
  the run function is called.

  The return value of the run function becomes the exit code of the thread.
*/
thread::thread (std::function<unsigned int ()> func)
  : shouldKill (true)
  , started (event::manual)
  , stat (state::ready)
  , exitcode (0)
  , stack (0)
  , thfunc (func)
{
  //setup SECURITY_ATTRIBUTES structure
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = false;
  initialize ();
  TRACE2 ("Created thread [%x]", id());
}

/*!
  Do the real work of creating and starting the thread
*/
void thread::initialize ()
{
  //create thread in suspended state
  HANDLE handle = (HANDLE)_beginthreadex (&sa, stack, (unsigned int (__stdcall *)(void*))entryProc, this,
                   CREATE_SUSPENDED, (UINT*)&id_);
  assert (handle);
  set_handle (handle);

  //Let thread run the initialization code. This will signal the "created"
  //semaphore than wait for "run" semaphore.
  ResumeThread (handle);

  //Wait for "created" semaphore to become signaled
  created.wait ();
}

/*!
  Destructor. Normally the thread should have ended earlier ( !isRunning )
  Anyhow we will use now brute force (TerminateThread) to end it.
*/
thread::~thread()
{
  TRACE2 ("Thread %s[%x] in destructor", name().c_str(), id_);
  if (shouldKill)
  {
    if (stat == state::running)
    {
      TRACE ("WARNING! thread was still running");
      TerminateThread (handle(), 0);
    }
    else if (stat == state::ready)
    {
      TRACE ("Terminating thread that was not started");
      TerminateThread (handle(), 0);
    }
  }
  id_ = 0;
}

/*!
  Static entry procedure for all threads. Assumes the passed argument is
  a thread object pointer and calls the virtual Run
*/
UINT _stdcall thread::entryProc (thread *th)
{
  th->created.signal();
  th->started.wait();
  try
  {
    if (th->init ())
    {
      th->stat = state::running;
      th->run ();
    }
    th->stat = state::ending;
    th->term ();
    th->stat = state::finished;
  }
  catch (...)
  {
    th->pex = std::current_exception ();
    TRACE ("Thread %s[%x] exception !!", th->name ().c_str (), th->id_);
  }

  TRACE2 ("Thread %s[%x] is ending", th->name().c_str(), th->id_);
  _endthreadex (th->exitcode);
  return th->exitcode;
}

/*!
  Default run function. Calls user supplied function if there is one
*/
void thread::run()
{
  if (thfunc)
    exitcode = thfunc ();
}

/*!
  Begin execution of a newly created thread
*/
void thread::start ()
{
  TRACE2 ("Thread %s[%x] is starting", name().c_str(), id_);
  assert (handle ());
  assert (stat == state::ready);
  started.signal ();
  stat = state::starting;
  Sleep (0);
}

/*!
  Wait for thread to finish execution

  \param time_limit time-out interval, in milliseconds
  \return `WAIT_OBJECT_0` thread finished
  \return `WAIT_TIMEOUT` time-out interval expired

  If an exception occurred during thread execution, it is re-thrown now.
*/
DWORD thread::wait (DWORD time_limit)
{
  DWORD ret = syncbase::wait (time_limit);
  if (ret == WAIT_OBJECT_0)
  {
    if (pex)
      std::rethrow_exception (pex);
  }
  return ret;
}

/*!
  Wait for thread to finish or an APC or IO completion routine to occur

  \param time_limit time-out interval, in milliseconds
  \return `WAIT_OBJECT_0` thread finished
  \return `WAIT_TIMEOUT` time-out interval expired
  \return `WAIT_IO_COMPLETION` wait ended by one or more APC-es queued.

  If an exception occurred during thread execution, it is re-thrown now.
*/
DWORD thread::wait_alertable (DWORD time_limit)
{
  DWORD ret = syncbase::wait_alertable (time_limit);
  if (ret == WAIT_OBJECT_0)
  {
    if (pex)
      std::rethrow_exception (pex);
  }
  return ret;
}

/*!
  Wait for thread to finish or a message to be queued

  \param time_limit time-out interval, in milliseconds
  \return `WAIT_OBJECT_0` thread finished
  \return `WAIT_TIMEOUT` time-out interval expired
  \return `WAIT_OBJECT_0+1` Input message received.

  If an exception occurred during thread execution, it is re-thrown now.
*/
DWORD thread::wait_msg (DWORD time_limit, DWORD mask)
{
  DWORD ret = syncbase::wait_msg (time_limit, mask);
  if (ret == WAIT_OBJECT_0)
  {
    if (pex)
      std::rethrow_exception (pex);
  }
  return ret;
}


} //end namespace

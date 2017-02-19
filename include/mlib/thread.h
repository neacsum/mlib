#pragma once
/*!
  \file thread.h thread class definition.

	(c) Mircea Neacsu 1999-2017
*/

#include "event.h"
#include <functional>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Wrapper for a Windows %thread
class thread : public syncbase
{
public:
                thread (int (*pfunc)(void *), void *arg=0, const char *name=0);
                thread (std::function<int (void*)> func, void *arg=0, const char *name=0);
  virtual       ~thread   ();
  virtual void  start     ();

  /// Return thread's id
  DWORD         id        () const              {return id_;};

  /// Return exit code 
  UINT          exitcode  () const              {return exitcode_; };

  /// Return \e true if %thread is running
  bool          is_running() const              {return running; };

  /// Return thread's priority
  int           priority  ();

  /// Change thread's priority
  void          priority  (int pri);

  /// Resume a suspended %thread
  virtual void  resume    ();

  /// Suspend a running %thread
  virtual void  suspend   ();

protected:
                thread (const char *name=0, bool inherit=false, DWORD stack_size=0, PSECURITY_DESCRIPTOR sd=NULL);

  /// Initialization function called before run
  virtual bool  init      ()                    {running = true; return true;};

  /// Finalization function called after run
  virtual bool  term      ()                    {running = false; return true;};

  /// Thread's body
  virtual void  run       ();

  UINT exitcode_;         ///< exit code
//  const char *name_;      ///< thread's name

private:
  thread (HANDLE handle, DWORD id);     ///< ctor used by current %thread.
  void initialize ();
  thread& operator= (const thread& t);  ///< not implemented - threads cannot be copied
  thread (const thread& t);             ///< not implemented - threads cannot be assigned

  DWORD id_;
  bool volatile running;
  bool shouldKill;
  event created, started;
  static UINT _stdcall entryProc( thread *ts );
  friend class current_thread;
  SECURITY_ATTRIBUTES sa;
  DWORD stack;
  std::function<int (void*)> thfunc;
  void *arg_;
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
  thread::resume;
  thread::suspend;

protected:
  void run() {};
};


//inlines

/// Return priority of a thread
inline
int thread::priority() {return GetThreadPriority (handle());};

/// Set priority of a thread
inline
void thread::priority (int pri) {SetThreadPriority (handle(), pri);};

#ifdef MLIBSPACE
};
#endif

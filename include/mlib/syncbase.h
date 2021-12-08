/*!
  \file syncbase.h syncbase class definition.

  (c) Mircea Neacsu 1999
*/
#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

#ifndef _INC_WINDOWS
#include <windows.h>
#endif
#include <string>
#include <atomic>
#include <vector>
#include <assert.h>

namespace mlib {

/// Base class for all named synchronization objects
class syncbase
{
public:
  syncbase ();
  syncbase (const syncbase& e);
  syncbase (syncbase&& e) noexcept;
  virtual ~syncbase ();

  syncbase& operator = (const syncbase& rhs);
  syncbase& operator = (syncbase&& rhs) noexcept;
  int operator == (const syncbase& rhs) const;

  virtual DWORD wait (DWORD time_limit=INFINITE);
  virtual DWORD wait_alertable (DWORD time_limit=INFINITE);
  virtual DWORD wait_msg (DWORD time_limit=INFINITE, DWORD mask=QS_ALLINPUT);
  virtual operator bool ();
  virtual bool try_wait ();

  /// Return OS handle of this object
  HANDLE handle () const { return hl->handle_; };

  /// Return object's name
  virtual const std::string& name () const { return name_; };
  
  /// Sets object's name
  virtual void name (const char* nam);

protected:
  syncbase (const char *name);        //protected constructor
  void set_handle (HANDLE h);

private:
  struct handle_life {
    HANDLE handle_;
    std::atomic_int lives;
  } *hl;
  std::string name_;
};

/*!
  Try to wait on the object.

  Returns \b true if the object was in a signaled state.
*/
inline
bool syncbase::try_wait ()
{
  return (wait(0) == WAIT_OBJECT_0);
}

/*!
  Wait for multiple objects

  Wrapper for [WaitForMultipleObjects](https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects)
  Windows API function.
  \param  objs    array of objects to wait for
  \param  count   number of objects in \p array
  \param  all     if true wait for all objects to become signaled
  \param  time_limit  time limit in milliseconds

  \ingroup syncro
*/
template <typename T>
DWORD multiwait (T* objs, int count, bool all=true, DWORD time_limit = INFINITE)
{
  assert (count < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  for (int i = 0; i < count; i++)
    harr[i] = objs[i].handle ();

  DWORD result = WaitForMultipleObjects (count, harr, all, time_limit);
  return result;
}

/*!
  Wait for multiple objects

  Wrapper for [WaitForMultipleObjects](https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects)
  Windows API function.
  \param  objs   vector of objects to wait for
  \param  all    if true wait for all objects to become signaled
  \param  time_limit  time limit in milliseconds

  \ingroup syncro
*/
template <typename T>
DWORD multiwait (std::vector<T>& objs, bool all=true, DWORD time_limit = INFINITE)
{
  assert (objs.size() < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  for (int i = 0; i < objs.size(); i++)
    harr[i] = objs[i].handle ();

  DWORD result = WaitForMultipleObjects ((DWORD)objs.size(), harr, all, time_limit);
  return result;
}

/*!
  Wait for multiple objects or a message to be queued

  Wrapper for [MsgWaitForMultipleObjects](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-msgwaitformultipleobjects)
  Windows API function.
  \param  objs    array of objects to wait for
  \param  count   number of objects in \p array
  \param  all     if true wait for all objects to become signaled
  \param  time_limit  time limit in milliseconds
  \param  mask    message mask (combination of QS_... constants)

  \ingroup syncro
*/
template <typename T>
DWORD multiwait_msg (T* objs, int count, bool all=true, DWORD time_limit = INFINITE, DWORD mask = QS_ALLINPUT)
{
  assert (count < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  for (int i = 0; i < count; i++)
    harr[i] = objs[i].handle ();

  DWORD result = MsgWaitForMultipleObjects (count, harr, all, time_limit, mask);
  return result;
}

/*!
  Wait for multiple objects or a message to be queued

  Wrapper for [MsgWaitForMultipleObjects](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-msgwaitformultipleobjects)
  Windows API function.
  \param  objs    array of objects to wait for
  \param  all     if true wait for all objects to become signaled
  \param  time_limit  time limit in milliseconds
  \param  mask    message mask (combination of QS_... constants)

  \ingroup syncro
*/
template <typename T>
DWORD multiwait_msg (std::vector<T>& objs, bool all = true, DWORD time_limit = INFINITE, DWORD mask = QS_ALLINPUT)
{
  assert (objs.size() < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  for (int i = 0; i < objs.size(); i++)
    harr[i] = objs[i].handle ();

  DWORD result = MsgWaitForMultipleObjects ((DWORD)objs.size(), harr, all, time_limit, mask);
  return result;
}

void udelay (unsigned short usec);

}

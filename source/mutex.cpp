/*!
  \file mutex.cpp mutex class implementation

  (c) Mircea Neacsu 1999

*/
#ifndef UNICODE
#define UNICODE
#endif

#include <mlib/mutex.h>
#include <assert.h>
#include <utf8/utf8.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif
/*!
  \class mutex
  \brief Wrapper for Windows mutexes.
  \ingroup syncro

  Only one %thread can acquire the object using a 'wait' function. Any
  other threads trying to acquire the %mutex will be blocked until the object
  is freed using the 'signal' function.
*/

///Constructor
mutex::mutex (const char *name) :
  syncbase (name)
{
  HANDLE h=CreateMutexW (NULL, FALSE, name?utf8::widen(name).c_str():NULL);
  assert (h);
  set_handle (h);
}

/*!
  Check if %mutex is free (signaled).

  If the %mutex is acquired (i.e. it was free) it is released back immediately.
*/
mutex::operator bool ()
{
  bool result = syncbase::operator bool ();
  if (result) signal ();
  return result;
}
#ifdef MLIBSPACE
};
#endif

/*!
  \file mutex.cpp mutex class implementation

  (c) Mircea Neacsu 1999

*/
#include <mlib/mutex.h>
#include <assert.h>

#if __has_include(<utf8/utf8.h>)
#define HAS_UTF8
#include <utf8/utf8.h>
#endif

namespace mlib {
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
#ifdef HAS_UTF8
  HANDLE h=CreateMutexW (NULL, FALSE, name?utf8::widen(name).c_str():NULL);
#else
  HANDLE h = CreateMutexA (NULL, FALSE, name);
#endif
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

}

/*!
  \file mutex.cpp mutex class implementation

  (c) Mircea Neacsu 1999

*/
#include <mlib/mlib.h>
#pragma hdrstop
#include <assert.h>

#if __has_include(<utf8/utf8.h>)
#define MLIB_HAS_UTF8_LIB
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

/// Constructor
mutex::mutex (const std::string& name)
  : syncbase (name)
{
#ifdef MLIB_HAS_UTF8_LIB
  HANDLE h = CreateMutexW (NULL, FALSE, !name.empty () ? utf8::widen (name).c_str () : NULL);
#else
  HANDLE h = CreateMutexA (NULL, FALSE, !name.empty () ? name.c_str () : NULL);
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
  if (result)
    signal ();
  return result;
}

} // namespace mlib

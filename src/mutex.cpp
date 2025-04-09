/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

#include <mlib/mlib.h>
#pragma hdrstop
#include <assert.h>

#include <utf8/utf8.h>

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
  HANDLE h = CreateMutexW (NULL, FALSE, 
    !name.empty () ? utf8::widen (name).c_str () : NULL);
  assert (h);
  set_handle (h);
}

/*!
  Check if mutex is free (signaled).

  If the mutex is acquired (i.e. it was free) it is released back immediately.
*/
mutex::operator bool ()
{
  bool result = syncbase::operator bool ();
  if (result)
    signal ();
  return result;
}

} // namespace mlib

/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file syncbase.h Definition of mlib::syncbase class.

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include "safe_winsock.h"

#include <string>
#include <atomic>
#include <vector>
#include <chrono>
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

  syncbase& operator= (const syncbase& rhs);
  syncbase& operator= (syncbase&& rhs) noexcept;
  int operator== (const syncbase& rhs) const;

  virtual void wait ();

  /// Wait a number of milliseconds for the object to become signaled.
  virtual DWORD wait (DWORD limit_msec);

  /// Wait a number of milliseconds for the object to become signaled.
  virtual DWORD wait (std::chrono::milliseconds limit);

  virtual DWORD wait_alertable (DWORD limit_msec = INFINITE);

  /// Wait for object to become signaled or a message to be queued
  virtual DWORD wait_msg (DWORD limit_msec = INFINITE, DWORD mask = QS_ALLINPUT);
  operator bool ();
  virtual bool is_signaled ();

  /// Return OS handle of this object
  HANDLE handle () const
  {
    return hl->handle_;
  };

  /// Return object's name
  virtual const std::string& name () const
  {
    return name_;
  };

protected:
  syncbase (const std::string& name); // protected constructor
  void set_handle (HANDLE h);
  virtual void name (const std::string& nam);

private:
  struct handle_life
  {
    HANDLE handle_;
    std::atomic_int lives;
  }* hl;
  std::string name_;
};

/// Change object's name
inline void syncbase::name (const std::string& nam)
{
  name_ = nam;
}

/// Wait for object to become signaled
inline void syncbase::wait ()
{
  assert (hl && hl->handle_);
  WaitForSingleObject (hl->handle_, INFINITE);
}

/*!
   \param limit_msec maximum wait time in milliseconds
   \return `WAIT_OBJECT0` if object becomes signaled
   \return `WAIT_TIMEOUT` if timeout has expired
*/
inline DWORD syncbase::wait (DWORD limit_msec)
{
  assert (hl && hl->handle_);
  return WaitForSingleObject (hl->handle_, limit_msec);
}

/*!
   \param limit maximum wait time
   \return `WAIT_OBJECT0` if object becomes signaled
   \return `WAIT_TIMEOUT` if timeout has expired
*/
inline DWORD syncbase::wait (std::chrono::milliseconds limit)
{
  assert (hl && hl->handle_);
  auto limit_msec = limit.count ();
  assert (0 < limit_msec && limit_msec < INFINITE); // must be a 32 bit value
  return WaitForSingleObject (hl->handle_, (DWORD)limit_msec);
}

/// Wait for object to become signaled or an APC or IO completion routine
/// to occur
inline DWORD syncbase::wait_alertable (DWORD limit_msec)
{
  assert (hl && hl->handle_);
  return WaitForSingleObjectEx (hl->handle_, limit_msec, TRUE);
}

/*!

  \param limit_msec   time-out interval, in milliseconds
  \param mask         input message types

  \return `WAIT_OBJECT_0`     thread finished
  \return `WAIT_TIMEOUT`      time-out interval expired
  \return `WAIT_OBJECT_0+1`   Input message received.

  The \p mask parameter can be any combination of flags described for the
  [GetQueueStatus](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getqueuestatus)
  function.
*/
inline DWORD syncbase::wait_msg (DWORD limit_msec, DWORD mask)
{
  return MsgWaitForMultipleObjects (1, &hl->handle_, FALSE, limit_msec, mask);
}

/// Check if object is signaled
inline syncbase::operator bool ()
{
  return is_signaled ();
}

/*!
  Try to wait on the object.

  Returns \b true if the object was in a signaled state.
*/
inline bool syncbase::is_signaled ()
{
  return (WaitForSingleObject (hl->handle_, 0) == WAIT_OBJECT_0);
}

/*!
  Wait for multiple objects until <u>all</u> become signaled.

  Wrapper for
  [WaitForMultipleObjects](https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects)
  Windows API function.
  \param  objs    array of objects to wait for
  \param  count   number of objects in \p array
  \param  msec    time limit (in milliseconds)

  \ingroup syncro
*/
template <typename T>
DWORD wait_all (const T* objs, int count, DWORD msec = INFINITE)
{
  assert (count < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  for (int i = 0; i < count; i++)
    harr[i] = objs[i]->handle ();

  DWORD result = WaitForMultipleObjects (count, harr, true, msec);
  return result;
}

/*!
  Wait for multiple objects until <u>all</u> become signaled.

  Wrapper for
  [WaitForMultipleObjects](https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects)
  Windows API function.
  \param  objs    objects to wait for
  \param  msec    time limit (in milliseconds)

  \ingroup syncro
*/
template <typename T>
DWORD wait_all (std::initializer_list<const T*> objs, DWORD msec = INFINITE)
{
  assert (objs.size () < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  int i = 0;
  for (auto& p : objs)
    harr[i++] = p->handle ();

  DWORD result = WaitForMultipleObjects ((DWORD)objs.size (), harr, true, msec);
  return result;
}

/*!
  Wait for multiple objects until <u>all</u> become signaled.

  Wrapper for
  [WaitForMultipleObjects](https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects)
  Windows API function.
  \param  objs    objects to wait for
  \param  limit   time limit in milliseconds

  \ingroup syncro
*/
template <typename T>
DWORD wait_all (std::initializer_list<const T*> objs, std::chrono::milliseconds limit)
{
  assert (objs.size () < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  int i = 0;
  for (auto& p : objs)
    harr[i++] = p->handle ();

  DWORD msec = (DWORD)limit.count ();
  DWORD result = WaitForMultipleObjects ((DWORD)objs.size (), harr, true, msec);
  return result;
}

/*!
  Wait for multiple objects until <u>any</u> of them becomes signaled.

  Wrapper for
  [WaitForMultipleObjects](https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects)
  Windows API function.
  \param  objs    array of objects to wait for
  \param  count   number of objects in \p array
  \param  msec    timeout interval (in milliseconds)

  \ingroup syncro
*/
template <typename T>
DWORD wait_any (const T* objs, int count, DWORD msec = INFINITE)
{
  assert (count < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  for (int i = 0; i < count; i++)
    harr[i] = objs[i].handle ();

  DWORD result = WaitForMultipleObjects (count, harr, false, msec);
  return result;
}

/*!
  Wait for multiple objects until <u>any</u> of them becomes signaled.

  Wrapper for
  [WaitForMultipleObjects](https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects)
  Windows API function.
  \param  objs    objects to wait for
  \param  msec    timeout interval (in milliseconds)

  \ingroup syncro
*/
template <typename T>
DWORD wait_any (std::initializer_list<const T*> objs, DWORD msec = INFINITE)
{
  assert (objs.size () < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  int i = 0;
  for (auto& p : objs)
    harr[i++] = p->handle ();

  DWORD result = WaitForMultipleObjects ((DWORD)objs.size (), harr, false, msec);
  return result;
}

/*!
  Wait for multiple objects until <u>any</u> of them becomes signaled.

  Wrapper for
  [WaitForMultipleObjects](https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects)
  Windows API function.
  \param  objs    objects to wait for
  \param  timeout timeout interval

  \ingroup syncro
*/
template <typename T>
DWORD wait_any (std::initializer_list<const T*> objs, std::chrono::milliseconds timeout)
{
  assert (objs.size () < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  int i = 0;
  for (auto& p : objs)
    harr[i++] = p->handle ();
  DWORD msec = (DWORD)timeout.count ();
  DWORD result = WaitForMultipleObjects ((DWORD)objs.size (), harr, false, msec);
  return result;
}

/*!
  Wait for multiple objects or a message to be queued

  Wrapper for
  [MsgWaitForMultipleObjects](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-msgwaitformultipleobjects)
  Windows API function.
  \param  objs    array of objects to wait for
  \param  count   number of objects in \p array
  \param  all     if `true`, wait for all objects to become signaled
  \param  msec    timeout interval (in milliseconds)
  \param  mask    message mask (combination of QS_... constants)

  \ingroup syncro
*/
template <typename T>
DWORD wait_msg (const T* objs, int count, bool all = true, DWORD msec = INFINITE,
                DWORD mask = QS_ALLINPUT)
{
  assert (count < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  for (int i = 0; i < count; i++)
    harr[i] = objs[i]->handle ();

  DWORD result = MsgWaitForMultipleObjects (count, harr, all, msec, mask);
  return result;
}

/*!
  Wait for multiple objects or a message to be queued

  Wrapper for
  [MsgWaitForMultipleObjects](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-msgwaitformultipleobjects)
  Windows API function.
  \param  objs    objects to wait for
  \param  all     if `true`, wait for all objects to become signaled
  \param  msec    timeout interval (in milliseconds)
  \param  mask    message mask (combination of QS_... constants)

  \ingroup syncro
*/
template <typename T>
DWORD wait_msg (std::initializer_list<const T*> objs, bool all = true, DWORD msec = INFINITE,
                DWORD mask = QS_ALLINPUT)
{
  assert (objs.size () < MAXIMUM_WAIT_OBJECTS);
  HANDLE harr[MAXIMUM_WAIT_OBJECTS];
  int i = 0;
  for (auto& p : objs)
    harr[i++] = p->handle ();

  DWORD result = MsgWaitForMultipleObjects ((DWORD)objs.size (), harr, all, msec, mask);
  return result;
}

} // namespace mlib

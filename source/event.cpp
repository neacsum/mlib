/*!
  \file event.cpp event class implementation.
  (c) Mircea Neacsu 1999

*/

#ifndef UNICODE
#define UNICODE
#endif

#include <mlib/event.h>
#include <assert.h>

#if __has_include(<utf/utf8.h>)
#define HAS_UTF8
#include <utf8/utf8.h>
#endif

namespace mlib {

/*!
  \class event
  \ingroup syncro

  Events have only two states: signaled and not signaled.
  An automatic event is set by signal and reset the release of any waiting
  thread. A manual event has to be explicitly reset. When an event is signaled
  all waiting threads are released. To release only one thread use 'pulse'
  function.
*/

///Constructor for event objects
event::event (mode md, bool signaled, const char *name) :
  syncbase (name),
  m (md)
{
#ifdef HAS_UTF8
  HANDLE h = CreateEventW (NULL, m==manual, signaled, name?utf8::widen(name).c_str():0);
#else
  HANDLE h = CreateEventA (NULL, m == manual, signaled, name);
#endif
  assert (h);
  set_handle (h);
}

/// Assignment operator
event& event::operator =(const event& rhs)
{
  syncbase::operator = (rhs);
  m = rhs.m;
  return *this;
}


/*!
  Check if event is signaled.

  Automatic events are set back to signaled state because testing resets them.
*/
event::operator bool ()
{
  bool result = syncbase::operator bool();
  if (result && m==automatic ) signal();
  return result;
}

}

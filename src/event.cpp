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
  \class event
  \ingroup syncro

  Events have only two states: signaled and not signaled.
  An automatic event is set by signal and reset the release of any waiting
  thread. A manual event has to be explicitly reset. When an event is signaled
  all waiting threads are released. To release only one thread use 'pulse'
  function.
*/

/// Constructor for event objects
event::event (bool manual, bool signaled, const std::string& name)
  : syncbase (name)
{
  HANDLE h = CreateEventW (NULL, manual, signaled, 
    !name.empty () ? utf8::widen (name).c_str () : NULL);
  assert (h);
  set_handle (h);
}

} // namespace mlib

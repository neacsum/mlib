#pragma once
/*
  MLIB Library
  (c) Mircea Neacsu 2001-2023. Licensed under MIT License.
  See README file for full license terms.
*/
///  \file event.h event class definition.

#include "syncbase.h"

namespace mlib {

class event : public syncbase
{
public:
  explicit event (bool manual, bool signaled = false, const std::string& name = std::string ());

  /// Set event to signaled state
  void signal ()
  {
    SetEvent (handle ());
  };

  /// Pulse event so that only one waiting thread is released
  void pulse ()
  {
    PulseEvent (handle ());
  };

  /// Set event to non-signaled state
  void reset ()
  {
    ResetEvent (handle ());
  };
};

/// Event objects that need manual reset
class manual_event : public event
{
public:
  explicit manual_event (bool signaled = false, const std::string& name = std::string ())
    : event (true, signaled, name){};
};

/// Event objects that reset automatically after a successful wait
class auto_event : public event
{
public:
  explicit auto_event (bool signaled = false, const std::string& name = std::string ())
    : event (false, signaled, name){};

  /// Check if event is signaled
  bool is_signaled () override;
};

/*!
  Automatic events are set back to signaled state because testing resets them.
*/
inline bool auto_event::is_signaled ()
{
  bool result = syncbase::is_signaled ();
  if (result)
    signal ();
  return result;
}

}; // namespace mlib

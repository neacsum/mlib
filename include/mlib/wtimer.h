/*!
  \file wtimer.h definition of waitable timer class

  (c) Mircea Neacsu 1999
*/
#pragma once

#include "syncbase.h"

namespace mlib {

class wtimer : public syncbase
{
public:
  /// Timer mode
  enum mode
  {
    manual,
    automatic
  };

  wtimer (mode m = automatic, const std::string& name = std::string (), bool use_apc = false);
  void start (DWORD interval_ms, DWORD period_ms = 0);
  void at (FILETIME& utctime, DWORD period_ms = 0);
  void stop ();

protected:
  /*!
    Function called for timers that use the APC feature.
  */
  virtual void at_timer (DWORD loval, DWORD hival) {};

private:
  bool apc_;
  static void CALLBACK timerProc (wtimer* obj, DWORD loval, DWORD hival);
};

} // namespace mlib

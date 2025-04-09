/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///   \file wtimer.h definition of mlib::wtimer timer class

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

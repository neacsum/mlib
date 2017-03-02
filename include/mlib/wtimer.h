#pragma once
/*!
  \file wtimer.h definition of timer class

	(c) Mircea Neacsu 1999
*/

#include "syncbase.h"

namespace mlib {

class timer : public syncbase
{
public:
  ///Timer mode
  enum mode { manual, automatic };

  timer (mode m=automatic, const char *name=NULL, bool use_apc=false);
  void start (DWORD interval_ms, DWORD period_ms=0);
  void at (FILETIME& utctime, DWORD period_ms=0);
  void stop ();

protected:
  /*!
    Function called for timers that use the APC feature.
  */
  virtual void at_timer (DWORD loval, DWORD hival) {};

private:
  bool apc_;
  static void CALLBACK timerProc (timer *obj, DWORD loval, DWORD hival);
};

} //namespace

/*!
  \file stopwatch.h Definition of stopwatch class

  (c) Mircea Neacsu 2017
*/
#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

namespace mlib {

/// Simple stopwatch timer (yet another one!)
class stopwatch
{
public:
  stopwatch ();
  void start ();
  void stop ();
  double msecLap ();
  double msecEnd ();

private:
  static LARGE_INTEGER freq;
  LARGE_INTEGER tbeg, tend;
};

}

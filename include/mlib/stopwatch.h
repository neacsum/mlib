/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///   \file stopwatch.h Definition of mlib::stopwatch class

#pragma once

#if __has_include("defs.h")
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

} // namespace mlib

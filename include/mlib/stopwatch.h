#pragma once
/*!
  \file STOPWATCH.H Definition of stopwatch class

  (c) Mircea Neacsu 2017
*/
#include "defs.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

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
#ifdef MLIBSPACE
};
#endif

/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///   \file stopwatch.h Definition of mlib::stopwatch class

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <chrono>

namespace mlib {

/// Simple stopwatch timer (yet another one!)
class stopwatch
{
public:
  stopwatch ();
  void start ();
  void stop ();
  std::chrono::steady_clock::duration lap ();
  std::chrono::steady_clock::duration end ();

  double lap_msec ();
  double end_msec ();

private:
  typedef std::chrono::steady_clock::time_point tpoint;
  tpoint tbeg, tend;
};

inline 
stopwatch::stopwatch ()
{
}

/// Start the stopwatch
inline
void stopwatch::start ()
{
  tbeg = std::chrono::steady_clock::now ();
  tend = tpoint{};
}

/// Stop the stopwatch
inline
void stopwatch::stop ()
{
  tend = std::chrono::steady_clock::now ();
}


/*!
  Return elapsed time from start.

  The stopwatch continues to run
*/
inline
std::chrono::steady_clock::duration stopwatch::lap ()
{
  return (std::chrono::steady_clock::now () - tbeg);
}


/// Return total duration
inline 
std::chrono::steady_clock::duration stopwatch::end ()
{
  return (tend - tbeg);
}

/*!
  Return number of milliseconds elapsed from start.

  The stopwatch continues to run
*/
inline
double stopwatch::lap_msec ()
{
  return std::chrono::duration<double, std::milli> (lap ()).count();
}


/// Return total duration in milliseconds between start and stop
inline
double stopwatch::end_msec ()
{
  return std::chrono::duration<double, std::milli> (tend - tbeg).count ();
}


} // namespace mlib

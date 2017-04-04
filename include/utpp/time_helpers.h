#pragma once
/*!
  \file time_helpers.h - Definition of Timer class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/

#ifdef UNITTEST_MINGW
  #ifndef __int64
    #define __int64 long long
  #endif
#endif

namespace UnitTest {

/// An object that can be interrogated to get elapsed time
class Timer
{
public:
  Timer ();
  void Start ();
  int GetTimeInMs () const;

private:
  __int64 GetTime () const;
  __int64 startTime;
  static __int64 frequency;
};


void SleepMs (int ms);

}

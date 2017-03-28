#pragma once

#include "config.h"


#ifdef UNITTEST_MINGW
  #ifndef __int64
    #define __int64 long long
  #endif
#endif

namespace UnitTest {

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


namespace TimeHelpers
{
  void SleepMs (int ms);
}


}

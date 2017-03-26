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

  void* m_threadHandle;

#if defined(_WIN64)
  unsigned __int64 processAffinityMask;
#else
  unsigned long processAffinityMask;
#endif

  __int64 startTime;
  __int64 frequency;
};


namespace TimeHelpers
{
  void SleepMs (int ms);
}


}

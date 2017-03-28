#include <utpp/timehelpers.h>
#include <windows.h>

namespace UnitTest {

__int64 Timer::frequency = 0;

Timer::Timer ()
  : startTime (0)
{
  if (!frequency)
  {
    ::QueryPerformanceFrequency (reinterpret_cast<LARGE_INTEGER*>(&frequency));
  }
}

void Timer::Start ()
{
  startTime = GetTime ();
}

int Timer::GetTimeInMs () const
{
  __int64 elapsedTime = GetTime () - startTime;
  double seconds = double (elapsedTime) / double (frequency);
  return int (seconds * 1000.0f);
}

__int64 Timer::GetTime () const
{
  LARGE_INTEGER curTime;
  ::QueryPerformanceCounter (&curTime);
  return curTime.QuadPart;
}



void TimeHelpers::SleepMs (int const ms)
{
  ::Sleep (ms);
}

}

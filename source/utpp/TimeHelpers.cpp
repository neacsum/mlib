#include <utpp/timehelpers.h>
#include <windows.h>

namespace UnitTest {

Timer::Timer ()
  : startTime (0)
  , m_threadHandle (::GetCurrentThread ())
{
  DWORD_PTR systemMask;
  ::GetProcessAffinityMask (GetCurrentProcess (), &processAffinityMask, &systemMask);

  ::SetThreadAffinityMask (m_threadHandle, 1);
  ::QueryPerformanceFrequency (reinterpret_cast<LARGE_INTEGER*>(&frequency));
  ::SetThreadAffinityMask (m_threadHandle, processAffinityMask);
}

void Timer::Start ()
{
  startTime = GetTime ();
}

int Timer::GetTimeInMs () const
{
  __int64 const elapsedTime = GetTime () - startTime;
  double const seconds = double (elapsedTime) / double (frequency);
  return int (seconds * 1000.0f);
}

__int64 Timer::GetTime () const
{
  LARGE_INTEGER curTime;
  ::SetThreadAffinityMask (m_threadHandle, 1);
  ::QueryPerformanceCounter (&curTime);
  ::SetThreadAffinityMask (m_threadHandle, processAffinityMask);
  return curTime.QuadPart;
}



void TimeHelpers::SleepMs (int const ms)
{
  ::Sleep (ms);
}

}

/*!
  \file stopwatch.cpp Implementation of stopwatch class

  (c) Mircea Neacsu 2017
*/


#include <mlib/stopwatch.h>
#include <assert.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif
LARGE_INTEGER stopwatch::freq;


/// Constructor
stopwatch::stopwatch ()
{
  if (freq.QuadPart == 0)
    QueryPerformanceFrequency (&freq);

  tbeg.QuadPart = tend.QuadPart = 0;
}

/// Start the stopwatch
void stopwatch::start ()
{
  tend.QuadPart = 0;
  QueryPerformanceCounter (&tbeg);
}

/// Stop the stopwatch
void stopwatch::stop ()
{
  QueryPerformanceCounter (&tend);
}

/*!
  Return number of milliseconds elapsed from start.

  The stopwatch continues to run
*/
double stopwatch::msecLap ()
{
  LARGE_INTEGER tlap;
  QueryPerformanceCounter (&tlap);
  assert (tbeg.QuadPart);
  return (double)(tlap.QuadPart - tbeg.QuadPart) / freq.QuadPart*1000.;
}

/// Return total duration in milliseconds between start and stop
double stopwatch::msecEnd ()
{
  assert (tbeg.QuadPart);
  assert (tend.QuadPart);
  return (double)(tend.QuadPart - tbeg.QuadPart) / freq.QuadPart*1000.;
}

#ifdef MLIBSPACE
};
#endif

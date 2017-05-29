/*!
  \file stopwatch.cpp - Implementation of stopwatch class

  (c) Mircea Neacsu 2017
*/


#include <mlib/stopwatch.h>


#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif
LARGE_INTEGER stopwatch::freq;


stopwatch::stopwatch ()
{
  if (freq.QuadPart == 0)
    QueryPerformanceFrequency (&freq);

  tbeg.QuadPart = tend.QuadPart = 0;
}

void stopwatch::start ()
{
  tend.QuadPart = 0;
  QueryPerformanceCounter (&tbeg);
}

void stopwatch::stop ()
{
  QueryPerformanceCounter (&tend);
}

double stopwatch::msecLap ()
{
  LARGE_INTEGER tlap;
  QueryPerformanceCounter (&tlap);
  return (double)(tlap.QuadPart - tbeg.QuadPart) / freq.QuadPart*1000.;
}

double stopwatch::msecEnd ()
{
  return (double)(tend.QuadPart - tbeg.QuadPart) / freq.QuadPart*1000.;
}

#ifdef MLIBSPACE
};
#endif

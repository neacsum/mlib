/*!
  \file convert.cpp Conversion functions

  (c) Mircea Neacsu 2017
*/
#include <mlib/convert.h>

double deg_reduce (double value)
{
  double val = fmod (value, 360);
  if (val < 0)
    val += 360;
  return val;
}

double DMD2deg( double value )
{
  int deg;

  deg = (int) (value/100.);
  value -= (double)deg * 100.;
  return deg + value/60.;
}

double DMS2deg( double value )
{
  int deg, min;

  deg = (int)(value/10000.);
  value -= (double)deg * 10000.;
  min = (int)(value/100.);
  value -= (double)min * 100.;
  return deg + min/60. + value/3600.;
}

///Conversion from decimal degrees to degrees, minutes (DDMM.mmm)
double deg2DMD (double value)
{
  int deg = (int)value;
  return (value-deg)*60. + deg*100.;
}	

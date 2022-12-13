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


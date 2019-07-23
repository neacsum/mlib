/*!
  \file statpars.h Definition of statistical parameters calculator class

  (c) Mircea Neacsu 2019
*/
#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

#include <deque>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Calculator for statistical parameters of a distribution
class statpars {
public:
  statpars (int nmax = 0);
  void add (double val);
  void add (double *vals, int count);
  void clear ();
  int count ();
  double average ();
  double stdev ();
  double mad ();
  double variance ();
  double skewness ();
  double kurtosis ();

private:
  void calculate ();
  std::deque<double> values;
  int nmax;
  bool calc;
  double ave, adev, var, sdev, skew, kurt;
};

/// Add a value to the calculator
inline
void statpars::add (double val)
{
  if (nmax && values.size () == nmax)
    values.pop_front ();
  values.push_back (val);
  calc = false;
}

/// Add multiple values to calculator
inline
void statpars::add (double *vals, int count)
{
  for (int i = 0; i < count; i++)
  {
    if (nmax && values.size () == nmax)
      values.pop_front ();
    values.push_back (*vals++);
  }
  calc = false;
}

/// Return distribution's average (first order moment)
inline 
double statpars::average ()
{
  if (!calc)
    calculate ();
  return ave;
}

/// Return distribution's variance (2nd order moment)
inline
double statpars::variance ()
{
  if (!calc)
    calculate ();
  return var;
}

/// Return standard deviation (square root of variance)
inline
double statpars::stdev ()
{
  if (!calc)
    calculate ();
  return sdev;
}

/// Return average absolute deviation also called "mean absolute deviation" (MAD)
inline 
double statpars::mad ()
{
  if (!calc)
    calculate ();
  return adev;
}

/// Return skewness (3rd order moment)
inline
double statpars::skewness ()
{
  if (!calc)
    calculate ();
  return skew;
}

/// Return kurtosis (4th order moment)
inline
double statpars::kurtosis ()
{
  if (!calc)
    calculate ();
  return kurt;
}

/// Return number of samples
inline
int statpars::count ()
{
  return (int)values.size ();
}

/// Calculator reset
inline
void statpars::clear ()
{
  values.clear ();
  calc = false;
}

#ifdef MLIBSPACE
};
#endif


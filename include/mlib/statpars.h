/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file statpars.h Definition of statistical parameters calculator class

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <deque>

namespace mlib {

/// Calculator for statistical parameters of a distribution
class statpars
{
public:
  statpars (int nmax = 0);
  statpars (std::vector<double> vec);

  void add (double val);
  void add (double* vals, int count);
  void add (std::vector<double> v);
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
  double sum, adev, var, sdev, skew, kurt;
};

/// Add a value to the calculator
inline void statpars::add (double val)
{
  if (nmax && values.size () == nmax)
  {
    sum -= values.front ();
    values.pop_front ();
  }
  values.push_back (val);
  sum += val;
  calc = false;
}

/// Add multiple values to calculator
inline void statpars::add (double* vals, int count)
{
  for (int i = 0; i < count; i++)
    add (vals[i]);
}

/// Add a vector of values
inline void statpars::add (std::vector<double> vals)
{
  for (double v : vals)
    add (v);
}

/// Return distribution's average (first order moment)
inline double statpars::average ()
{
  if (!values.empty ())
    return sum / values.size ();
  else
    return 0.;
}

/// Return distribution's variance (2nd order moment)
inline double statpars::variance ()
{
  if (!calc)
    calculate ();
  return var;
}

/// Return standard deviation (square root of variance)
inline double statpars::stdev ()
{
  if (!calc)
    calculate ();
  return sdev;
}

/// Return average absolute deviation also called "mean absolute deviation" (MAD)
inline double statpars::mad ()
{
  if (!calc)
    calculate ();
  return adev;
}

/// Return skewness (3rd order moment)
inline double statpars::skewness ()
{
  if (!calc)
    calculate ();
  return skew;
}

/// Return kurtosis (4th order moment)
inline double statpars::kurtosis ()
{
  if (!calc)
    calculate ();
  return kurt;
}

/// Return number of samples
inline int statpars::count ()
{
  return (int)values.size ();
}

/// Calculator reset
inline void statpars::clear ()
{
  values.clear ();
  calc = false;
  sum = 0.;
}

} // namespace mlib

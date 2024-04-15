#pragma once

/*!
  \file   ipow.h Integer exponentiation function template

  (c) Mircea Neacsu 2019
*/

#if __has_include("defs.h")
#include "defs.h"
#endif
#include <assert.h>

namespace mlib {

/// Integer exponentiation function
template <typename T>
T ipow (T base, int exp)
{
  assert (exp >= 0);
  T result = 1;
  while (exp)
  {
    if (exp & 1)
      result *= base;
    exp >>= 1;
    base *= base;
  }
  return result;
}

/// Specialization for double, can handle negative exponents
template <>
inline double ipow (double base, int exp)
{
  if (exp < 0)
  {
    base = 1. / base;
    exp = -exp;
  }
  double result = 1;
  while (exp)
  {
    if (exp & 1)
      result *= base;
    exp >>= 1;
    base *= base;
  }
  return result;
}

} // namespace mlib

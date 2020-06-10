#pragma once

/*!
  \file   ipow.h Integer exponentiation function template

  (c) Mircea Neacsu 2019
*/
#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

namespace mlib {

///integer exponentiation function
template <typename T>
T ipow (T base, int exp)
{
  T result = 1;
  while (exp)
  {
    if (exp & 1)
      result *= base;
    exp >>= 1;
    base *= base;
  }
  return result;
};


};

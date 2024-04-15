#pragma once
/*!
  \file   poly.h Polynomial evaluation using Horner's scheme

  (c) Mircea Neacsu 2020
*/

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <array>
#include <vector>

namespace mlib {

template <typename T>
/*!
  Evaluate a polynomial using Horner's scheme
  \param x Evaluation point
  \param coeff polynomial coefficients in order from lowest power (coeff[0]) to
               highest power (coeff[N-1])
  \param n size of coefficient's array.

  \return Polynomial value in point x
          coeff[n-1]*x^(n-1) + coeff[n-2]*x^(n-2) + ... + coeff[1]*x + coeff[0]
*/
T poly (T x, const T* coeff, int n)
{
  T val = coeff[n - 1];
  for (int i = n - 2; i >= 0; i--)
  {
    val *= x;
    val += coeff[i];
  }
  return val;
}

/*!
  Evaluate a polynomial using Horner's scheme
  \param x Evaluation point
  \param coeff array of polynomial coefficients in order from lowest power
               (coeff[0]) to highest power (coeff[N-1])
  \return Polynomial value in point x
          coeff[N-1]*x^(N-1) + coeff[N-2]*x^(N-2) + ... + coeff[1]*x + coeff[0]

  This template function will generate a new instantiation for each array size.
*/
template <typename T, size_t N>
T poly (T x, std::array<T, N> coeff)
{
  return poly (x, coeff.data (), (int)N);
}

/*!
  Evaluate a polynomial using Horner's scheme
  \param x Evaluation point
  \param coeff vector of polynomial coefficients in order from lowest power
               (coeff[0]) to highest power (coeff[N-1])
  \return Polynomial value in point x
          coeff[N-1]*x^(N-1) + coeff[N-2]*x^(N-2) + ... + coeff[1]*x + coeff[0]
*/
template <typename T>
T poly (T x, std::vector<T> coeff)
{
  return poly (x, coeff.data (), (int)coeff.size ());
}

} // namespace mlib

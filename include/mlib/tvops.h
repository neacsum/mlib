/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file tvops.h Operations on timeval structure.

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include "safe_winsock.h"
#include <assert.h>
#include <ostream>
#include <chrono>

/// \addtogroup tvops
/// @{

timeval operator+ (const timeval& t1, const timeval& t2);
timeval operator- (const timeval& t1, const timeval& t2);
timeval& operator+= (timeval& lhs, const timeval& rhs);
timeval& operator-= (timeval& lhs, const timeval& rhs);
timeval operator* (const timeval& op1, int op2);
timeval operator* (int op1, const timeval& op2);
timeval operator/ (const timeval& op1, int op2);

/// Equality operator
inline bool operator== (const timeval& t1, const timeval& t2)
{
  if (t1.tv_sec == t2.tv_sec)
    return (t1.tv_usec == t2.tv_usec);
  return 0;
}

// MSVC - make sure you compile with /Zc:__cplusplus option
#if __cplusplus < 202002L
// C++11 or C++17

/// "Less than" operator
inline bool operator< (const timeval& t1, const timeval& t2)
{
  if (t1.tv_sec != t2.tv_sec)
    return (t1.tv_sec < t2.tv_sec);
  else
    return (t1.tv_usec < t2.tv_usec);
}

/// "Greater than" operator
inline bool operator> (const timeval& t1, const timeval& t2)
{
  if (t1.tv_sec != t2.tv_sec)
    return (t1.tv_sec > t2.tv_sec);
  else
    return (t1.tv_usec > t2.tv_usec);
}

/// "Greater or equal than" operator
inline bool operator>= (const timeval& t1, const timeval& t2)
{
  return !(t1 < t2);
}

/// "Less or equal than" operator
inline bool operator<= (const timeval& t1, const timeval& t2)
{
  return !(t1 > t2);
}

/// "Not equal" operator
inline bool operator!= (const timeval& t1, const timeval& t2)
{
  return !(t1 == t2);
}
#else
// for C++20 use spaceship operator

/// 'spaceship' operator
inline auto operator<=> (const timeval& t1, const timeval& t2)
{
  if (t1.tv_sec == t2.tv_sec)
    return (t1.tv_usec <=> t2.tv_usec);
  else
    return (t1.tv_sec <=> t2.tv_sec);
}

#endif

// conversion to/from microseconds
long long usec64 (const timeval& tv);
timeval fromusec (long long us);

// conversion to seconds
double secd (const timeval& tv);

#ifdef _WIN32
timeval fromsystime (const SYSTEMTIME& st);
void tosystime (const timeval& tv, SYSTEMTIME* st);
void tolocaltime (const timeval& tv, SYSTEMTIME* st);
timeval zone_bias ();
#endif

void normalize (timeval& tv);

timeval fromdouble (double d);

// Inline implementations ----------------------------------------------------

/// Addition operator
inline
timeval operator+ (const timeval& t1, const timeval& t2)
{
  timeval ans;
  ans.tv_usec = t1.tv_usec + t2.tv_usec;
  ans.tv_sec = t1.tv_sec + t2.tv_sec;
  normalize (ans);
  return ans;
}

/// Subtraction operator
inline
timeval operator- (const timeval& t1, const timeval& t2)
{
  timeval ans;

  ans.tv_usec = t1.tv_usec - t2.tv_usec;
  ans.tv_sec = t1.tv_sec - t2.tv_sec;
  normalize (ans);
  return ans;
}

/// Addition assignment operator
inline
timeval& operator+= (timeval& lhs, const timeval& rhs)
{
  lhs.tv_usec += rhs.tv_usec;
  lhs.tv_sec += rhs.tv_sec;
  normalize (lhs);
  return lhs;
}

/// Subtraction assignment
inline
timeval& operator-= (timeval& lhs, const timeval& rhs)
{
  lhs.tv_usec -= rhs.tv_usec;
  lhs.tv_sec -= rhs.tv_sec;
  normalize (lhs);
  return lhs;
}

/// Conversion to floating-point seconds
inline
double secd (const timeval& tv)
{
  return tv.tv_sec + (double)tv.tv_usec / 1000000;
}

/// Conversion to 64-bit microseconds
inline
long long usec64 (const timeval& tv)
{
  return (long long)tv.tv_sec * 1000000L + (long long)tv.tv_usec;
}

/// Conversion from 64-bit microseconds
inline
timeval fromusec (long long us)
{
  timeval tv;
  tv.tv_sec = (int)(us / 1000000L);
  tv.tv_usec = (int)(us % 1000000L);
  return tv;
}

/// Conversion from floating-point seconds
inline
timeval fromdouble (double d)
{
  timeval tv;
  tv.tv_sec = (long)d;
  tv.tv_usec = (long)((d - tv.tv_sec) * 1000000);
  return tv;
}

/// Conversion from a chrono duration
template <typename T>
timeval from_chrono (const T& dur)
{
  using namespace std::chrono;
  auto sec = duration_cast<seconds>(dur);
  auto us = duration_cast<microseconds> (dur - sec);
  return timeval{(long)sec.count (), (long)us.count ()};
}

/// Conversion to a chrono duration in microseconds
inline
std::chrono::microseconds to_chrono (const timeval& tv)
{
  using namespace std::chrono;
  return seconds (tv.tv_sec) + microseconds (tv.tv_usec);
}


/// Multiplication by an integer
inline
timeval operator* (const timeval& op1, int op2)
{
  timeval tv;

  tv.tv_usec = op1.tv_usec * op2;
  tv.tv_sec = op1.tv_sec * op2;
  normalize (tv);
  return tv;
}

/// Multiplication by an integer
inline
timeval operator* (int op1, const timeval& op2)
{
  return op2 * op1;
}

/// Division by an integer
inline
timeval operator/ (const timeval& op1, int op2)
{
  timeval tv;
  tv.tv_usec = op1.tv_usec / op2;
  tv.tv_sec = op1.tv_sec / op2;
  normalize (tv);
  return tv;
}

/// Stream extraction operator
inline ::std::ostream& operator<< (::std::ostream& os, const timeval& tv)
{
  os << "{ tv_sec: " << tv.tv_sec << ", tv_usec: " << tv.tv_usec << '}';
  return os;
}

  /// @}

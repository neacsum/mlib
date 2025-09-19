/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/


#include <mlib/mlib.h>
#pragma hdrstop
#include <assert.h>

// 100ns intervals between 1/1/1601 and 1/1/1970 as reported by SystemTimeToFileTime()
#define FILETIME_1970 0x019db1ded53e8000
#define ONE_SECOND    ((long)1000000)

/// \defgroup tvops Operations with timeval structures

#ifdef _WIN32
///  Convert from UTC system time to Unix time scale (UTC form 01/01/70)
timeval fromsystime (const SYSTEMTIME& st)
{
  __int64 ft;
  timeval tv;
  SystemTimeToFileTime (&st, (FILETIME*)&ft);
  ft -= FILETIME_1970;
  tv.tv_sec = (long)(ft / 10000000i64);
  tv.tv_usec = (long)((ft / 10i64) % 1000000i64);
  assert (tv.tv_usec > -ONE_SECOND && tv.tv_usec < ONE_SECOND);
  return tv;
}

///  Convert from timeval format to SYSTEMTIME format
void tosystime (const timeval& tv, SYSTEMTIME* st)
{
  assert (tv.tv_usec > -ONE_SECOND && tv.tv_usec < ONE_SECOND);
  __int64 ft = (__int64)tv.tv_sec * 10000000i64 + (__int64)tv.tv_usec * 10i64;
  ft += FILETIME_1970;
  FileTimeToSystemTime ((FILETIME*)&ft, st);
}

/// Return time zone bias
timeval zone_bias ()
{
  static timeval bias = {0, 1};
  if (bias.tv_usec != 0)
  {
    TIME_ZONE_INFORMATION tz;
    DWORD ret = GetTimeZoneInformation (&tz);
    if (ret == TIME_ZONE_ID_STANDARD)
      bias.tv_sec = 60 * (tz.StandardBias + tz.Bias);
    else if (ret == TIME_ZONE_ID_DAYLIGHT)
      bias.tv_sec = 60 * (tz.DaylightBias + tz.Bias);
    else if (ret == TIME_ZONE_ID_UNKNOWN)
      bias.tv_sec = 60 * tz.Bias;
    bias.tv_usec = 0;
  }
  return bias;
}

/// Convert to local time (in a SYSTEMTIME structure)
void tolocaltime (const timeval& tv, SYSTEMTIME* st)
{
  assert (tv.tv_usec > -ONE_SECOND && tv.tv_usec < ONE_SECOND);
  __int64 ft = (__int64)tv.tv_sec * 10000000i64 + (__int64)tv.tv_usec * 10i64;
  ft += FILETIME_1970;
  ft -= (__int64)zone_bias ().tv_sec * 10000000i64;
  FileTimeToSystemTime ((FILETIME*)&ft, st);
}
#endif

/*!
  Following an arithmetic operation, brings timeval structure to a canonical
  form where tv_usec is less than 1000000 and has the same sign as tv_sec member.
*/
void normalize (timeval& tv)
{
  if (tv.tv_usec >= ONE_SECOND)
  {
    do
    {
      tv.tv_sec++;
      tv.tv_usec -= ONE_SECOND;
    } while (tv.tv_usec >= ONE_SECOND);
  }
  else if (tv.tv_usec <= -ONE_SECOND)
  {
    do
    {
      tv.tv_sec--;
      tv.tv_usec += ONE_SECOND;
    } while (tv.tv_usec <= -ONE_SECOND);
  }

  if (tv.tv_sec >= 1 && tv.tv_usec < 0)
  {
    tv.tv_sec--;
    tv.tv_usec += ONE_SECOND;
  }
  else if (tv.tv_sec < 0 && tv.tv_usec > 0)
  {
    tv.tv_sec++;
    tv.tv_usec -= ONE_SECOND;
  }
  assert (tv.tv_usec > -ONE_SECOND && tv.tv_usec < ONE_SECOND);
}


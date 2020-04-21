#pragma once

#include <windows.h>
#include <assert.h>
/*
      TVOPS.H - operations with timeval structures
*/

timeval  operator +(const timeval& t1, const timeval& t2);
timeval  operator -(const timeval& t1, const timeval& t2);
timeval& operator +=(timeval& lhs, const timeval& rhs);
timeval& operator -=(timeval& lhs, const timeval& rhs);
timeval  operator *(const timeval& op1, int op2);
timeval  operator *(int op1, const timeval& op2);
timeval  operator /(const timeval& op1, int op2);
int      operator <(const timeval& t1, const timeval& t2);
int      operator ==(const timeval& t1, const timeval& t2);
int      operator !=(const timeval& t1, const timeval& t2);

//conversion to/from milliseconds since midnight
void tolocal (const timeval& tv, DWORD *msSinceMidnight, WORD *usec=NULL);
void fromlocal (DWORD ms, WORD usec, timeval& tv);

//conversion to/from microseconds
LONGLONG usec64 (const timeval& tv);
timeval fromusec (LONGLONG us);

//conversion to seconds
double secd (const timeval& tv);

timeval fromsystime (const SYSTEMTIME& st);
void tosystime (const timeval& tv, SYSTEMTIME* st);
void tolocaltime (const timeval& tv, SYSTEMTIME* st);
int zone_bias();

void normalize (timeval& tv);

timeval fromdouble (double d);

// Inline implementations ----------------------------------------------------
inline timeval operator +(const timeval& t1, const timeval& t2)
{
  timeval ans;
  ans.tv_usec = t1.tv_usec + t2.tv_usec;
  ans.tv_sec = t1.tv_sec + t2.tv_sec;
  normalize (ans); 
  return ans;
}

inline timeval operator -(const timeval& t1, const timeval& t2)
{
  timeval ans;
  
  ans.tv_usec = t1.tv_usec - t2.tv_usec;
  ans.tv_sec = t1.tv_sec - t2.tv_sec;
  normalize (ans);
  return ans;
}

inline timeval& operator += (timeval& lhs, const timeval& rhs)
{
  lhs.tv_usec += rhs.tv_usec;
  lhs.tv_sec += rhs.tv_sec;
  normalize (lhs);
  return lhs;
}

inline timeval& operator -= (timeval& lhs, const timeval& rhs)
{
  lhs.tv_usec -= rhs.tv_usec;
  lhs.tv_sec -= rhs.tv_sec;
  normalize (lhs);
  return lhs;
}

inline double secd (const timeval& tv)
{
  return tv.tv_sec + (double)tv.tv_usec/1000000;
}

inline LONGLONG usec64 (const timeval& tv)
{
  return (LONGLONG)tv.tv_sec * 1000000L + (LONGLONG)tv.tv_usec;
}

inline timeval fromusec (LONGLONG us)
{
  timeval tv;
  tv.tv_sec = (int)(us / 1000000L);
  tv.tv_usec = (int)(us % 1000000L);
  return tv;
}

inline timeval fromdouble (double d)
{
  timeval tv;
  tv.tv_sec = (long)d;
  tv.tv_usec = (long)((d-tv.tv_sec)*1000000);
  return tv;
}

inline timeval operator *(const timeval& op1, int op2)
{
  timeval tv;

  tv.tv_usec = op1.tv_usec * op2;
  tv.tv_sec = op1.tv_sec * op2;
  normalize (tv);
  return tv;
}

inline timeval operator *(int op1, const timeval& op2)
{
  return op2*op1;
}

inline timeval operator /(const timeval& op1, int op2)
{
  timeval tv;
  tv.tv_usec = op1.tv_usec / op2;
  tv.tv_sec = op1.tv_sec / op2;
  normalize (tv);
  return tv;
}

inline int operator <(const timeval& t1, const timeval& t2)
{
  if (t1.tv_sec != t2.tv_sec)
    return (t1.tv_sec < t2.tv_sec);
  else
    return  (t1.tv_usec < t2.tv_usec);
}

inline int operator ==(const timeval& t1, const timeval& t2)
{
  if (t1.tv_sec == t2.tv_sec)
    return  (t1.tv_usec == t2.tv_usec);
  return 0;
}

inline int operator !=(const timeval& t1, const timeval& t2)
{
  return !operator==(t1, t2);
}

#pragma comment (lib, "hybase.lib")

#include <stdio.h>

#include <mlib/tvops.h>

//100ns intervals between 1/1/1601 and 1/1/1970 as reported by SystemTimeToFileTime()
#define FILETIME_1970     0x019db1ded53e8000
#define ONE_SECOND ((long)1000000)
/*
  Convert from UTC system time to Unix time scale (UTC form 01/01/70)
*/
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

/*
  Convert from timeval format to SYSTEMTIME format
*/
void tosystime (const timeval& tv, SYSTEMTIME* st)
{
  assert (tv.tv_usec > -ONE_SECOND && tv.tv_usec < ONE_SECOND);
  __int64 ft = (__int64)tv.tv_sec * 10000000i64 + (__int64)tv.tv_usec*10i64;
  ft += FILETIME_1970;
  FileTimeToSystemTime ((FILETIME*)&ft, st);
}

static timeval bias = {0,1};

static void get_bias()
{
   TIME_ZONE_INFORMATION tz;
   DWORD ret = GetTimeZoneInformation( &tz );
   if (ret == TIME_ZONE_ID_STANDARD)
      bias.tv_sec = 60 * (tz.StandardBias + tz.Bias);
   else if (ret == TIME_ZONE_ID_DAYLIGHT)
      bias.tv_sec = 60 * (tz.DaylightBias + tz.Bias);
   else if (ret == TIME_ZONE_ID_UNKNOWN)
      bias.tv_sec = 60 * tz.Bias;
   bias.tv_usec = 0;
}

int zone_bias()
{
  if (bias.tv_usec != 0)
     get_bias();
  return bias.tv_sec;
}

void tolocaltime (const timeval& tv, SYSTEMTIME* st)
{
  assert (tv.tv_usec > -ONE_SECOND && tv.tv_usec < ONE_SECOND);
  __int64 ft = (__int64)tv.tv_sec * 10000000i64 + (__int64)tv.tv_usec*10i64;
  ft += FILETIME_1970;
  if (bias.tv_usec != 0)
     get_bias();
  ft -= (__int64)bias.tv_sec * 10000000i64;
  FileTimeToSystemTime ((FILETIME*)&ft, st);
}

/*
  Convart from local time milliseconds from midnight to Unix time UTC
*/
void fromlocal (DWORD ms, WORD usec, timeval& tv)
{
  static DWORD last_ms = 86500000;
  static timeval ut_midnight;

  if (ms < last_ms)
  {
    //if time since midnight has wrapped around recalculate Unix time at midnight
    SYSTEMTIME lt;
    __int64 ft;
    GetLocalTime (&lt);
    lt.wHour = lt.wMinute = lt.wSecond = lt.wMilliseconds = 0;
    SystemTimeToFileTime (&lt, (FILETIME *)&ft);
    ft -= FILETIME_1970;
    ut_midnight.tv_sec = (long)(ft / 10000000i64);
    ut_midnight.tv_usec =(long)((ft / 10i64) % 1000000i64);
  }
  last_ms = ms;

  //find bias of local time if not known already
  if (bias.tv_usec != 0)
     get_bias();

  tv.tv_sec = ms/1000;
  tv.tv_usec = (ms % 1000) * 1000 + usec;
  tv += bias;
  tv += ut_midnight;
  
  normalize (tv);
}

/*
  Convart from Unix time UTC to local time milliseconds from midnight
*/
void tolocal (const timeval& tv, DWORD* msSinceMidnight, WORD* usec)
{
  assert (tv.tv_usec > -ONE_SECOND && tv.tv_usec < ONE_SECOND);

  if (bias.tv_usec != 0)
    get_bias ();

  timeval temp = tv - bias;
  *msSinceMidnight = ((temp.tv_sec % 86400) * 1000) + temp.tv_usec/1000;
  if (usec)
    *usec = (WORD)(temp.tv_usec % 1000);
}

void normalize (timeval& tv)
{
  if (tv.tv_usec >= ONE_SECOND)
  {
    do 
    {
      tv.tv_sec++;
      tv.tv_usec -= ONE_SECOND;
    }
    while (tv.tv_usec >= ONE_SECOND);
  }
  else if (tv.tv_usec <= -ONE_SECOND) 
  {
    do 
    {
      tv.tv_sec--;
      tv.tv_usec += ONE_SECOND;
    }
    while (tv.tv_usec <= -ONE_SECOND);
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

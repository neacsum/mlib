/*!
  \file dprintf.cpp Implementation of dprintf() function

  (c) Mircea Neacsu 1999-2020. All rights reserved.
*/

#include <mlib/mlib.h>
#pragma hdrstop
#include <stdio.h>
#include <stdarg.h>

static INIT_ONCE dpr_init = INIT_ONCE_STATIC_INIT;
static CRITICAL_SECTION dpr_cs;

// Initialization function executed by InitOnceExecuteOnce
BOOL CALLBACK dpr_inifun (PINIT_ONCE, PVOID, PVOID*)
{
  InitializeCriticalSection (&dpr_cs);
  return true;
}

/*!
  printf style function writes messages using OutputDebugString.
  Message length is limited to MAX_DPRINTF_CHARS characters.

  \param fmt print format
  \return true if successful, false otherwise

  \note Function is thread-safe. Calls from different threads are serialized by
  a critical section object.
*/

bool dprintf (const char* fmt, ...)
{
  char buffer[MAX_DPRINTF_CHARS];
  size_t sz;
  int r;
  wchar_t* out;
  InitOnceExecuteOnce (&dpr_init, dpr_inifun, NULL, NULL);

  va_list params;
  va_start (params, fmt);
  sprintf_s (buffer, "[%x] ", GetCurrentThreadId ());
  sz = sizeof (buffer) - strlen (buffer) - 2; // reserve space for final \0 and \n
  r = vsnprintf (buffer + strlen (buffer), sz, fmt, params);
  va_end (params);
  if (r < 0)
    return false; // for some reason we cannot print
  if ((size_t)r > sz - 1)
    buffer[MAX_DPRINTF_CHARS - 2] = 0; // print was truncated
  strcat_s (buffer, "\n");

  int wsz = MultiByteToWideChar (CP_UTF8, 0, buffer, -1, 0, 0);
  if (wsz && (out = (wchar_t*)malloc (wsz * sizeof (wchar_t))))
  {
    MultiByteToWideChar (CP_UTF8, 0, buffer, -1, out, wsz);
    EnterCriticalSection (&dpr_cs);
    OutputDebugString (out);
    LeaveCriticalSection (&dpr_cs);
    free (out);
    return true;
  }
  return false;
}

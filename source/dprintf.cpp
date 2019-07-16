/*!
  \file dprintf.cpp Implementation of dprintf() function

  (c) Mircea Neacsu 1999-2000. All rights reserved.
*/
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <mlib/dprintf.h>

/*!
  printf style function writes messages using OutputDebugString.
  Message length is limited to MAX_DPRINTF_CHARS characters.

  \param fmt print format
  \return true if successful, false otherwise
*/

bool dprintf (const char *fmt, ...)
{
  char buffer[MAX_DPRINTF_CHARS];
  size_t sz;
  int r;
  wchar_t *out;

  va_list params;
  va_start (params, fmt);
  sprintf (buffer, "[%x] ", GetCurrentThreadId ());
  sz = sizeof (buffer) - strlen (buffer) - 2; //reserve space for final \0 and \n
  r = vsnprintf (buffer + strlen (buffer), sz, fmt, params);
  va_end (params);
  if (r < 0)
    return false; // for some reason we cannot print
  if ((size_t)r > sz - 1)
    buffer[MAX_DPRINTF_CHARS - 2] = 0; // print was truncated
  strcat (buffer, "\n");

  int wsz = MultiByteToWideChar (CP_UTF8, 0, buffer, -1, 0, 0);
  if (wsz && (out = (wchar_t*)malloc (wsz*sizeof (wchar_t))))
  {
    MultiByteToWideChar (CP_UTF8, 0, buffer, -1, out, wsz);
    OutputDebugString (out);
    free (out);
    return true;
  }
  return false;
}


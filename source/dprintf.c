#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <mlib/dprintf.h>

//*********************************************************
// Function		: dprintf
//
// Description	: printf style function will write messages
//		using OutputDebugString for GUI based applications.
//		Message length is limited to 1024 characters.
//
// Parameters:
//		char *fmt	- print format
//		...			- variable arguments
//*********************************************************
void dprintf (const char *fmt, ...)
{
  char buffer[1024];
  size_t sz;
#ifdef UNICODE
  int nsz, wsz;
  wchar_t *out;
#endif
  va_list params;
  va_start (params, fmt);
  sprintf (buffer, "[%x] ", GetCurrentThreadId ());
  sz = sizeof (buffer) - strlen (buffer) - 3;
  vsnprintf (buffer + strlen (buffer), sz, fmt, params);
  strcat (buffer, "\n");
#ifndef UNICODE
  OutputDebugString (buffer);
#else
  nsz = (int)strlen (buffer);
  wsz = MultiByteToWideChar (CP_UTF8, 0, buffer, nsz, 0, 0);
  out = (wchar_t*)calloc (wsz + 1, sizeof (wchar_t));
  if (wsz)
    MultiByteToWideChar (CP_UTF8, 0, buffer, nsz, &out[0], wsz);
  out[wsz] = 0;
  OutputDebugString (out);
  free (out);
#endif
}


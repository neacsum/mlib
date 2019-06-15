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
  static char buffer[1024];
  static wchar_t out[1024];
  size_t sz;
  va_list params;
  va_start (params, fmt);
  sprintf (buffer, "[%x] ", GetCurrentThreadId ());
  sz = sizeof (buffer) - strlen (buffer) - 3;
  vsnprintf (buffer + strlen (buffer), sz, fmt, params);
  strcat (buffer, "\n");

  MultiByteToWideChar (CP_UTF8, 0, buffer, -1, out, _countof(out));
  OutputDebugStringW (out);
}


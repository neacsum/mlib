/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This is part of MLIB project. See LICENSE file for full license terms.
*/

#include <mlib/mlib.h>
#pragma hdrstop
#include <stdio.h>
#include <stdarg.h>
#include <thread>
#include <mutex>
#include <sstream>
#include <cstring>

namespace mlib {

static std::mutex dpr_lock;

/*!  

  On Windows platform, the function  uses the `OutputDebugString` API call to
  generate the output. On other platforms, output is sent to `stderr`.

  Message length is limited to `MAX_DPRINTF_CHARS` characters.

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
  va_list params;
  va_start (params, fmt);
#ifdef _WIN32
  sprintf_s (buffer, "[%x] ", GetCurrentThreadId ());
#else
  std::stringstream s;
  s << '[' << std::this_thread::get_id () << ']';
  strcpy (buffer, s.str ().c_str ());
#endif
  sz = sizeof (buffer) - strlen (buffer) - 2; // reserve space for final \0 and \n
  r = vsnprintf (buffer + strlen (buffer), sz, fmt, params);
  va_end (params);
  if (r < 0)
    return false; // for some reason we cannot print
  if ((size_t)r > sz - 1)
    buffer[MAX_DPRINTF_CHARS - 2] = 0; // print was truncated
#ifdef _WIN32
  strcat_s (buffer, "\n");
  wchar_t* out;
//  std::lock_guard<std::mutex> l (dpr_lock);

  int wsz = MultiByteToWideChar (CP_UTF8, 0, buffer, -1, 0, 0);
  if (wsz && (out = (wchar_t*)malloc (wsz * sizeof (wchar_t))))
  {
    MultiByteToWideChar (CP_UTF8, 0, buffer, -1, out, wsz);
    OutputDebugString (out);
    free (out);
    return true;
  }
  return false;
#else
  strcat (buffer, "\n");
  std::lock_guard<std::mutex> l (dpr_lock);
  fputs (buffer, stderr);
  return true;
#endif
}


} // namespace mlib

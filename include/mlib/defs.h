#pragma once

/* inet_ntoa triggers some deprecation warnings. Disable them for now. */
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#ifndef _WINSOCK2API_
#ifdef _WINSOCKAPI_
#error <winsock.h> has been included before <winsock2.h>
#else
#include <winsock2.h>
#endif
#endif



#ifndef NODEFAULTLIB
#pragma comment (lib, "mlib.lib")
#endif

/// log facility claimed by MLIB
#define MLIB_LOGFAC (25<<3)

/// set to 1 to send trace output to syslog
#define MLIB_SYSLOG_TRACE 1

/*!
  Active trace level. A rough description would be:
  - 9 = a lot
  - 8 = many informational messages
  - 2 = only significant events
  - 0 or undefined = errors only

  Intermediate levels are left available for user programs
*/
#ifndef MLIB_TRACE_LEVEL
#define MLIB_TRACE_LEVEL 7
#endif
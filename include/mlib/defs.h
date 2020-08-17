#pragma once

/* inet_ntoa triggers some deprecation warnings. Disable them for now. */
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <winsock2.h>


#ifndef NODEFAULTLIB
#pragma comment (lib, "mlib.lib")
#endif

/// log facility claimed by MLIB
#define MLIB_LOGFAC (25<<3)

/// send trace output to syslog
//#define MLIB_SYSLOG_TRACE

/*!
  Active trace level. A rough description would be:
  - 9 = a lot
  - 8 = many informational messages
  - 2 = only significant events
  - 0 or undefined = errors only

  Intermediate levels are left available for user programs
*/
//#define MLIB_TRACE_LEVEL 2

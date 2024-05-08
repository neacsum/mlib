#pragma once

#if defined(_MSC_VER) && !defined(NODEFAULTLIB)
#pragma comment(lib, "mlib.lib")
#endif

/// log facility claimed by MLIB
#define MLIB_LOGFAC (25 << 3)

/// set to 1 to send trace output to syslog
// #define MLIB_SYSLOG_TRACE 1

/*!
  Active trace level. A rough description would be:
  - 9 = a lot
  - 8 = many informational messages
  - 2 = only significant events
  - 0 or undefined = errors only

  Intermediate levels are left available for user programs
*/

// If _TRACE has a numeric value, that becomes the trace level
#if !defined(MLIB_TRACE_LEVEL) && defined(_TRACE) && ((_TRACE) + 0 > 0)
#define MLIB_TRACE_LEVEL _TRACE
#endif

#ifndef MLIB_TRACE_LEVEL
#define MLIB_TRACE_LEVEL 7
#endif

#define SQLITE_ENABLE_COLUMN_METADATA

//Standard types
typedef unsigned long DWORD;
typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef float FLOAT;

typedef int INT;
typedef unsigned int UINT;

#include <stdint.h>
#include <stddef.h>

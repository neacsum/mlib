#pragma once
/*!
  \file log.h syslog related functions.

  (c) Mircea Neacsu 2008

  The header file is a shameless copy of the original BSD syslog.h file,
  however implementations are new.

*/
#if __has_include ("defs.h")
#include "defs.h"
#endif

#include <winsock2.h>

// facility codes 0 to 24 are defined in syslog.h
#define	LOG_USER	(1<<3)	///< facility for random user-level messages

/*!
  \name Priorities

  priorities/facilities are encoded into a single 32-bit quantity, where the
  bottom 3 bits are the priority (0-7) and the top 28 bits are the facility
  (0-big number). This is the mapping of priority codes.
  \{
*/
#define  LOG_EMERG    0   ///< system is unusable
#define  LOG_ALERT    1   ///< action must be taken immediately
#define  LOG_CRIT     2   ///< critical conditions
#define  LOG_ERR      3   ///< error conditions
#define  LOG_WARNING  4   ///< warning conditions
#define  LOG_NOTICE   5   ///< normal but significant condition
#define  LOG_INFO     6   ///< informational
#define  LOG_DEBUG    7   ///< debug-level messages
///\}

/*!

\{
  \name Masks and Operations
  The following definitions can be used for operations on priorities and
  facilities. See syslog() and setlogmask() for examples.
*/
#define  LOG_PRIMASK  0x7       ///< mask to extract priority part
#define  LOG_FACMASK  0x03f8    ///< mask to extract facility part
#define  LOG_MASK(pri)  (1 << (pri))            ///< mask for one priority
#define  LOG_UPTO(pri)  ((1 << ((pri)+1)) - 1)  ///< all priorities through pri
/// Used for arguments to syslog
#define  LOG_MAKEPRI(fac, pri)  ((fac) | (pri))
///\}

/*!
  \name Flags for openlog
  The following flags can be specified in the opt argument of openlog():
\{
 */
#define LOGOPT_PID        0x01    ///< log the process id with each message
#define LOGOPT_OUTDEBUG   0x02    ///< log to Windows debug port (OutputDebugString)
#define LOGOPT_NOUDP      0x04    ///< do not send UDP data
#define LOGOPT_FILE       0x08    ///< log to disk file
///\}

#define LOG_PORT     514  /* port number for logger (same as SYSLOG service) */

#ifdef __cplusplus
extern "C" {
#endif

/// Close connection to logger.
void closelog (void);

/// Open connection to logger.
void openlog (const char *ident, int option, int facility);

/// Set the log mask level.
int setlogmask (int mask);

/// Set option flags
int setlogopt (int opt);

/// Generate a log message using FMT string and option arguments. 
void syslog (int facility_priority, char *fmt, ...);

/// Generate a log message at debug level using FMT string and option arguments. 
void syslog_debug (const char *fmt, ...);

extern int log_defaultopt;
extern int log_defaultmask;
extern char log_servhostname[_MAX_PATH];
extern char log_fname[_MAX_PATH];

#ifdef __cplusplus
}
#endif

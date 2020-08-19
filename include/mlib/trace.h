/*!
  \file trace.h Definition of trace debugging macros.

  (c) Mircea Neacsu 1999-2002. All rights reserved.

  TRACE
  Usage: TRACE (fmt, arg1, arg2, ....);
  
  If _TRACE symbol is defined the macro produces a
  printf-like string to the debug output.
  Otherwise TRACE has no effect.
  
  There are also 9 additional macros TRACEx (where x is 1 to 9) that behave
  like TRACE if _TRACE_LEVEL is greater than x. The intended usage is
  to have very detailed trace information if _TRACE_LEVEL is high with
  diminishing amounts of information as _TRACE_LEVEL is lowered.
*/
#pragma once

#include "dprintf.h"

# undef TRACE
# undef TRACE1
# undef TRACE2
# undef TRACE3
# undef TRACE4
# undef TRACE5
# undef TRACE6
# undef TRACE7
# undef TRACE8
# undef TRACE9

//SYSLOG_TRACE forces TRACE
#if MLIB_SYSLOG_TRACE && !defined (MLIB_TRACE)
#define MLIB_TRACE
#endif

# if defined (MLIB_TRACE_LEVEL)
// only some traces enabled (up to and including MLIB_TRACE_LEVEL)
#  define __TRL(A) (A > MLIB_TRACE_LEVEL)? 0 : TRACE
#  define TRACE1 __TRL(1)
#  define TRACE2 __TRL(2)
#  define TRACE3 __TRL(3)
#  define TRACE4 __TRL(4)
#  define TRACE5 __TRL(5)
#  define TRACE6 __TRL(6)
#  define TRACE7 __TRL(7)
#  define TRACE8 __TRL(8)
#  define TRACE9 __TRL(9)
# else
// all traces are enabled
#  define TRACE1 TRACE
#  define TRACE2 TRACE
#  define TRACE3 TRACE
#  define TRACE4 TRACE
#  define TRACE5 TRACE
#  define TRACE6 TRACE
#  define TRACE7 TRACE
#  define TRACE8 TRACE
#  define TRACE9 TRACE
# endif

#ifdef MLIB_TRACE
# if MLIB_SYSLOG_TRACE
#  include "log.h"
#  define TRACE syslog_debug
# else
#   define TRACE dprintf
# endif
#else
# define TRACE 1? 0 : dprintf
#endif

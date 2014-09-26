#if !defined( TRACE_H )
#define TRACE_H

/*  TRACE.H
    (c) Mircea Neacsu 1999-2002. All rights reserved.

    Definition of trace debugging macros.


	  TRACE
    Usage: TRACE(  fmt, arg1, arg2, .... );

    If _TRACE symbol is defined the macro produces a
    printf-like string to the debug output.
    Otherwise TRACE has no effect.

    There are also 9 additional macros TRACEx (where x is 1 to 9) that behave
    like TRACE if _TRACE_LEVEL is greater than x. The intended usage is
    to have very detailed trace information if _TRACE_LEVEL is high with
    diminishing amounts of information as _TRACE_LEVEL is lowered.
*/
#ifdef _TRACE
# undef TRACE
# if defined (_TRACE_SYSLOG)
#  include "log.h"
#  define TRACE syslog_debug
# else
#   define TRACE dprintf
# endif
# if defined (_TRACE_LEVEL)
#  define __TRL(A) (A > _TRACE_LEVEL)? (void)0 : TRACE
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
#else
# undef TRACE
# define TRACE 1? (void)0 : dprintf
# define TRACE1 1? (void)0 : dprintf
# define TRACE2 1? (void)0 : dprintf
# define TRACE3 1? (void)0 : dprintf
# define TRACE4 1? (void)0 : dprintf
# define TRACE5 1? (void)0 : dprintf
# define TRACE6 1? (void)0 : dprintf
# define TRACE7 1? (void)0 : dprintf
# define TRACE8 1? (void)0 : dprintf
# define TRACE9 1? (void)0 : dprintf
#endif

#include "dprintf.h"

#endif
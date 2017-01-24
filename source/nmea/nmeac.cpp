/**
  \file NMEAC.CPP 
  \brief Implementation of NMEA parsing functions.

  (c) Coastal Oceanographics Inc. 1994-2004.
  (c) HYPACK Inc. 2004-2006.

  The first argument to all parsing functions is the string to be parsed.
  Remaining arguments are pointers to parsed values. If any of them is NULL
  the corresponding field is parsed but not returned.

  The return value is 0 if a parsing error occurs. Where the NMEA sentence
  description has changed between different versions of standard (NMEA 2.1,
  NMEA 3.0) the return value is the maximum standard version number that
  matches the input.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nmeac.h"


/**
    token - Return next token of a NMEA sentence. 
    
    Tokens are delimited by ',', \<CR\>, '*' or end of string.
    We cannot use strtok directly because we want to leave the string unchanged
    and we don't want to skip over consecutive empy fields.

*/

char *token (parse_context& ctx)
{
  char *ret;
  if (ctx.saved)
  {
    *ctx.toparse = ctx.saved;
    if (ctx.saved != ',')
      return NULL;
    ctx.toparse++;
  }
  ret = ctx.toparse;
  while (*ctx.toparse && *ctx.toparse != ',' && *ctx.toparse != '\r' && *ctx.toparse != '*')
    ctx.toparse++;

  ctx.saved = *ctx.toparse;
  *ctx.toparse = 0;
  return ret;
}

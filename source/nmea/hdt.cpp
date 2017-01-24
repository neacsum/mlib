#include "nmeac.h"
#include <mlib/convert.h>

/**
  HDT sentence
    $ttHDT,xxx.x,T

  \note
    This sentence is obsolete
*/
int hdt (const char *buf, double *head)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "HDT", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (head, atof (tok)*D2R);
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'T')
    return 0;
  return 2;
}

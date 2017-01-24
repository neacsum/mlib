#include "nmeac.h"
#include <mlib/convert.h>
/**
  HDM sentence.

  $ttHDM,xxx.x,M

  \note
    This sentence is obsolete
*/
int hdm (const char *buf, double *head)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "HDM", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (head, atof (tok)*D2R);
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'M')
    return 0;
  return 2;
}

#include "nmeac.h"

/**
  DPT sentence.
  
  $ttDPT,dep,offset,range
*/
int dpt (const char *buf, double *depth, double *offset, double *range)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "DPT", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (depth, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (offset, atof (tok));
  NEXT_TOKEN (tok, 2);
  IFPAR (range, atof (tok));
  return 3;
}

#include "nmeac.h"

/**
  ZDA sentence.

  $ttZDA,time,day,month,year,loch,locm

  UTC = local +loch + locm/60
*/
int zda (const char *buf, double *time, unsigned short *day, unsigned short *month, unsigned short *year)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "ZDA", 3))
    return 0;

  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  if (!strlen(tok))
    return 0;
  IFPAR (day, (unsigned short)atoi (tok));
  NEXT_TOKEN (tok, 0);
  if (!strlen(tok))
    return 0;
  IFPAR (month, (unsigned short)atoi (tok));
  NEXT_TOKEN (tok, 0);
  if (!strlen(tok))
    return 0;
  IFPAR (year, (unsigned short)atoi (tok));
  return 3;
}

#include "nmeac.h"
#include <mlib/convert.h>

/**
  VTG sentence.

  $ttVTG,true,T,mag,M,knots,N,kph,K,mode

  True heading takes precedence over magnetic heading. Speed value in knots
  takes precedence over value in km/h.
*/
int vtg (const char *buf, double *speed, double *head)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  bool th, sk;
  th = sk = false;
  if (!tok || memcmp (tok+3, "VTG", 3))
    return 0;

  NEXT_VALIDTOKEN (tok, 0);
  if (head)
  {
    *head = atof (tok)*D2R;
    th = true;
  }
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'T')
    return 0;
  NEXT_TOKEN (tok, 0);
  if (head && !th && *tok)
    *head = atof (tok)*D2R;
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'M')
    return 0;
  NEXT_TOKEN (tok, 0);
  if (speed && *tok)
  {
    *speed = atof (tok);
    sk = true;
  }
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'N')
    return 0;
  NEXT_TOKEN (tok, 0);
  if (speed && !sk && *tok)
    *speed = atof (tok)*0.539957;
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'K')
    return 0;
  return 3;
}


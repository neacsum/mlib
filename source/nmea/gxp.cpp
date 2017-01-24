#include "nmeac.h"
#include <mlib/convert.h>
/**
  GXP sentence.

  $ttGXP,time,lat,N,lon,a,wp

  \note
    This sentence is obsolete.
*/
int gxp (const char *buf, double *lat, double *lon, double *time, int *wp)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "GXP", 3))
    return 0;

  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (lat, DMD2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lat && *tok && *tok == 'S') *lat *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (lon, DMD2rad(atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lon && *tok && *tok == 'W' ) *lon *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (wp, atoi(tok));
  return 1;
}

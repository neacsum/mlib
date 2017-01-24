#include "nmeac.h"
#include <mlib/convert.h>
/**
  GNS sentence.

  $--GNS,hhmmss.ss,llll.lll,a,yyyyy.yyy,a,c--c,xx,x.x,x.x,x.x,x.x,x.x*hh

  \return
    version of standard to which sentence conforms or 0 if invalid statement.
  \note
    Height is ellipsoidal.
*/
int gns (const char *buf,double *time, double *lat, double *lon, int *mode, int *sat,
          double *dop, double *height, double *age, int *station)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "GNS", 3))
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
  IFPAR (mode, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (sat, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (dop, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (height, atof (tok));
  NEXT_TOKEN (tok, 0);    //'M'
  NEXT_TOKEN (tok, 0);
  if (height)
  {
    double undul = atof(tok);
    *height += undul;
  }
  NEXT_TOKEN (tok, 0);  //'M'
  NEXT_TOKEN (tok, 2);
  IFPAR (age, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (station, atoi (tok));
  return 3;
}


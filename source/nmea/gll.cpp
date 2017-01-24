#include "nmeac.h"
#include <mlib/convert.h>

/**
  GLL sentence.

  $ttGLL,lat,N,lon,W[,time,valid,[mode]]

  \return
    - 0 if invalid statement
    - 1 old NMEA 1.x standard (missing time and mode)
    - 2 newer version 2 (missing mode field)
    - 3 version 3
*/
int gll (const char *buf, double *lat, double *lon, double *time, int *mode)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "GLL", 3))
    return 0;

  NEXT_TOKEN (tok, 0);
  IFPAR (lat, DMD2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lat && *tok && *tok == 'S') *lat *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (lon, DMD2rad(atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lon && *tok && *tok == 'W' ) *lon *= -1.;
  NEXT_TOKEN (tok, 1);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  NEXT_TOKEN (tok, 2);
  if (mode)
  {
    if (*tok == 'A') *mode = 1;
    else if(*tok == 'D') *mode = 2;
    else *mode = 0;
  }
  return 3;
}


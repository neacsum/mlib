#include "nmeac.h"
#include <mlib/convert.h>
/**
  RMC sentence.

  $ttRMC,time,stat,lat,N/S,lon,E/W,speed,head,date,magvar,E/W,mode
*/
int rmc (const char *buf, double *lat, double *lon, double *time, double *speed, double *head, int *date, int *mode)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "RMC", 3))
    return 0;

  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);    //status
  if (*tok && *tok != 'A' && *tok != 'V')
    return 0;
  NEXT_TOKEN (tok, 0);
  IFPAR (lat, DMD2rad (atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lat && *tok && *tok == 'S') *lat *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (lon, DMD2rad(atof (tok)));
  NEXT_TOKEN (tok, 0);
  if (lon && *tok && *tok == 'W' ) *lon *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (speed, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (head, atof (tok)*D2R);
  NEXT_TOKEN (tok, 0);
  IFPAR (date, atoi (tok));
  NEXT_TOKEN (tok, 0);      //skip magnetic variation
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'E' && *tok != 'W')
    return 0;
  NEXT_TOKEN (tok, 2);
  IFPAR (mode, (*tok == 'A')?1:
               (*tok == 'D')?2:
               (*tok == 'P')?3:
               (*tok == 'R')?4:
               (*tok == 'F')?5:0);
  return 3;
}

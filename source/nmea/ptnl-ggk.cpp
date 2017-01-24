#include "nmeac.h"
#include <mlib/convert.h>

/**
  PTNLGGK sentence.

  $PTNL,GGK,hhmmss.ss,mmddyy,llll.ll,a,yyyyy.yy,a,x,xx,x.x,EHTxx.x,M*hh

  - hhmmss.ss   = UTC of Position Fix
  - mmddyy      = UTC date
  - llll.ll     = Latitude
  - a           = Hemisphere (N/S)
  - yyyyy.yy    = Longitude
  - a           = E/W
  - x           = GPS Quality Indicator
                  - 0 = fix not available or invalid
                  - 1 = No Realtime position, navigation fix
                  - 2 = Realtime position, ambiguities not fixed
                  - 3 = Realtime position, ambiguities fixed
  - xx          = Number of Satellites in use; common satellites between ref and rover,
                  values between 00 to 12; may be different from the number in view.
  - x.x         = PDOP
  - xx.x        = Ellipsoidal height.
                = Altitude above/below mean sea level for
                  position of marker. Note, if no orthometric
                  height is available the local ell. height will be
                  exported.
  - M           = Units of altitude meters (fixed text “M”)
  - hh          = Checksum

 \code
    $PTNL,GGK,172814.00,071296,3723.46587704,N,12202.26957864,W,3,06,1.7,EHT-6.777,M*48
 \endcode

*/
int ptnlggk (const char *buf, double *lat, double *lon, double *time, double *height,
             double *dop, int *sat, int *mode)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok, "$PTNL", 5))
    return 0;
  NEXT_TOKEN (tok, 0);
  if (memcmp (tok, "GGK", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);        //date
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
  NEXT_TOKEN (tok, 1);
  if (memcmp (tok, "EHT", 3))
    return 0;
  IFPAR (height, atof (tok+3));
  return 1;
}


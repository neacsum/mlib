#include "nmeac.h"

/**
  Leica LLQ sentence.

  $GPLLQ,hhmmss.ss,mmddyy,eeeeee.eee,M,nnnnnn.nnn,M,g,ss,q.q,z.z,M*hh
  - hhmmss.ss   UTC time of position
  - mmddyy      UTC date
  - eeeeee.ee   Grid Easting, meters
  - M           Meter (fixed text “M”)
  - nnnnnn.nn   Grid Northing, meters
  - M           Meter (fixed text “M”)
  - g           GPS Quality
                - 0 = fix not available or invalid
                - 1 = No Realtime position, navigation fix
                - 2 = Realtime position, ambiguities not fixed
                - 3 = Realtime position, ambiguities fixed
  - ss          Number of satellites used in computation
  - q.q         Coordinate Quality
  - z.z         Altitude above/below mean sea level for
                position of marker. Note, if no orthometric
                height is available the local ell. height will be
                exported.
  - M           Meter (fixed text “M”)
  - hh          Checksum
*/
int llq (const char *buf, double *time, double *x, double *y, int *mode, int *sat, double *dop, double *height)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "LLQ", 3))
    return 0;

  NEXT_TOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  NEXT_TOKEN (tok, 0);      //date
  IFPAR (x, atof (tok));
  NEXT_TOKEN (tok, 0);
  if (*tok !=  'M')
    return 0;
  NEXT_TOKEN (tok, 0);
  IFPAR (y, atof (tok));
  NEXT_TOKEN (tok, 0);
  if (*tok !=  'M')
    return 0;
  NEXT_TOKEN (tok, 0);
  IFPAR (mode, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (sat, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (dop, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (height, atof (tok));
  NEXT_TOKEN (tok, 0);
  if (*tok !=  'M')
    return 0;
  return 1;
}


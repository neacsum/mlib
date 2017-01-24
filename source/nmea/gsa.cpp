#include "nmeac.h"

/**
  GSA - GNSS DOP and Active Satellites
  
  GNSS receiver operating mode, satellites used in the navigation solution reported by the GGA or GNS
  sentence, and DOP values.
  
    $--GSA,a,x,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,x.x,x.x,x.x<CR><LF>
           | | |                                 |  |   |   |
           | | |                                 |  |   |   +-- VDOP
           | | |                                 |  |   +------ HDOP
           | | |                                 |  +---------- PDOP
           | | |_________________________________|          
           | |            +--- ID numbers of satellites used in solution
           | +---------------- Mode: 1 = Fix not available, 2 = 2D, 3 = 3D
           +------------------ Mode: M = Manual, forced to operate in 2D or 3D mode
                                     A = Automatic, allowed to automatically switch 2D/3D

 */

int gsa (const char *buf, int *hmode, int *fmode, int *sv, double *pdop, double *hdop, double *vdop)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "GSA", 3))
    return 0;

  NEXT_TOKEN (tok, 0);
  IFPAR (hmode, (*tok == 'A')?1:(*tok == 'M')?2:0);
  NEXT_TOKEN (tok, 0);
  IFPAR (fmode, atoi (tok));
  if (sv)
  {
    for (int i=0; i<12; i++)
    {
      NEXT_TOKEN (tok, 0);
      sv[i] = atoi(tok);
    }
  }
  NEXT_TOKEN (tok, 0);
  IFPAR (pdop, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (hdop, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (vdop, atof (tok));
  return 3;
}


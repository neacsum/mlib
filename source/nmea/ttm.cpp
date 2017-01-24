#include "nmeac.h"
#include <mlib/convert.h>

/**
  Tracked Target message.

  $xxTTM,num,dist,brg,relbrg,speed,course,relcog,cpa,tcpa,units,name,status,ref,hhmmss.ss,acq*hh
  - num = target number
  - dist = target distance relative to own ship
  - brg = target bearing
  - relbrg = 'T' for true bearing, 'R' for relative
  - speed = target speed
  - course = target course
  - relcog = true/relative course
  - cpa = distance to closest point of approach
  - tca = time to CPA
  - units = speed/distance units ('K' or 'N' or 'S')
  - name = target name
  - status = 'L' lost, 'Q' query, 'T' tracking
  - ref = referance target
  - hhmmss = UTC time
  - acq = acquisition status
*/

int ttm (const char *buf, double *utc, int *num, char *name, double *dist, double *brg, 
         int *relbrg, double *speed, double *cog, int *relcog, double *cpa, double *tcpa, 
         int *stat)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "TTM", 3))
    return 0;

  NEXT_TOKEN (tok, 0);
  if (atoi (tok)<0)
    return 0;   //garbage
  IFPAR (num, atoi (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (dist, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (brg, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (relbrg, (*tok=='R'?1:0));
  NEXT_TOKEN (tok, 0);
  IFPAR (speed, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (cog, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (relcog, (*tok=='R'?1:0));
  NEXT_TOKEN (tok, 0);
  IFPAR (cpa, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (tcpa, atof (tok));
  NEXT_TOKEN (tok, 0);
  if (*tok == 'K')
  {
    if (dist)
      *dist *= 1000;
    if (speed)
      *speed /= 3.6;
  }
  else if (*tok == 'N')
  {
    if (dist)
      *dist *= 1852;
    if (speed)
      *speed /= MPS2KNOT;
  }
  NEXT_TOKEN (tok, 0);
  if (name)
    strcpy (name, tok);
  NEXT_TOKEN (tok, 0);
  if (stat)
    *stat = (*tok == 'L')? 1 :
            (*tok == 'Q')? 2 : 0;
  NEXT_TOKEN (tok, 0);    //reference target;
  NEXT_TOKEN (tok, 0);
  IFPAR (utc, atof(tok));

  return 1;
}


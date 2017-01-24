#include "nmeac.h"

/**
  GSV - Satellites in View

  $xxGSV,t,n,c,id,el,az,sn,id,az,el,sn,id,az,el,sn,id,az,el,sn*hh
         | | | |  |  |  |  |          ||          ||__________+- satellite 4
         | | | |  |  |  |  |          ||__________+------------- satellite 3
         | | | |  |  |  |  |__________+------------------------- satellite 2
         | | | |  |  |  +------- SNR satellite 1 (0-99)
         | | | |  |  +---------- Azimuth satellite 1 (0-359)
         | | | |  +------------- Elevation satellite 1 (0-90)
         | | | +---------------- SV number satellite 1
         | | +------------------ Total number of satellites in view
         | +-------------------- Message number
         +---------------------- Total number of messages

*/

int gsv (const char *buf, int *tmsg, int *msg, int *count, int *sv, int *az, int *elev, int *snr)
{
  parse_context ctx(buf);

  int nmsg;
  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "GSV", 3))
    return 0;

  NEXT_TOKEN (tok, 0);
  IFPAR (tmsg, atoi (tok));
  NEXT_TOKEN (tok, 0);
  nmsg = atoi (tok)-1;
  if ( nmsg < 0 || nmsg > 8)
    return 0;
  IFPAR (msg, nmsg+1);
  NEXT_TOKEN (tok, 0);
  IFPAR (count, atoi (tok));
  for (int i=0; i<4; i++)
  {
    NEXT_TOKEN (tok, 1);
    if (sv) sv[nmsg*4+i] = atoi (tok);
    NEXT_TOKEN (tok, 0);
    if (elev) elev[nmsg*4+i] = atoi (tok);
    NEXT_TOKEN (tok, 0);
    if (az) az[nmsg*4+i] = atoi (tok);
    NEXT_TOKEN (tok, 0);
    if (snr) snr[nmsg*4+i] = atoi (tok);
  }

  return 1;
}


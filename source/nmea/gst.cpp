#include "nmeac.h"

/**
  GST sentence.

	$ttGST,time,rms,semimaj,semimin,orient,stdlat,stdlon,stdh
*/
int gst (const char *buf, double *time, double *rms, double *smaj, double *smin,
         double *orient, double *stdlat, double *stdlon, double *stdh)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "GST", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (rms, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (smaj, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (smin, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (orient, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (stdlat, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (stdlon, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (stdh, atof (tok));
  return 3;
}

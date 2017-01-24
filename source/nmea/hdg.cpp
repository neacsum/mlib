#include "nmeac.h"
#include <mlib/convert.h>

/**
  HDG sentence.

	$ttHDG,hdg,dev,E/W,var,E/W

    magnetic = hdg + dev
    true = magnetic + var
*/
int hdg (const char *buf, double *head, double *dev, double *var)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "HDG", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (head, atof (tok)*D2R);
  NEXT_TOKEN (tok, 0);
  IFPAR (dev, atof (tok)*D2R);
  NEXT_TOKEN (tok, 0);
  if (dev && *tok == 'W') *dev *= -1.;
  NEXT_TOKEN (tok, 0);
  IFPAR (var, atof (tok)*D2R);
  NEXT_TOKEN (tok, 0);
  if (var && *tok == 'W') *var *= -1.;
  return 3;
}


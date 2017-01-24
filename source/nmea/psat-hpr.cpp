#include "nmeac.h"
#include <mlib/convert.h>

/**
  Hemisphere GNSS proprietary sentence

  $PSAT,HPR,time,heading,pitch,roll,type

  type  N = GPS derived heading
        G = gyro heading
*/
int psathpr(const char *buf, double *time, double *head, double *pitch, double *roll, char *type)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok, "$PSAT", 5))
    return 0;
  NEXT_TOKEN (tok, 0);
  if (memcmp (tok, "HPR", 3))
    return 0;
  NEXT_VALIDTOKEN (tok, 0);
  IFPAR (time, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (head, atof (tok)*D2R);
  NEXT_TOKEN (tok, 0);
  if ( strlen(tok) )
     IFPAR (pitch, atof (tok));
  NEXT_TOKEN (tok, 0);
  if ( strlen(tok) )
     IFPAR (roll, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (type, tok[0]);
  return 3;
}

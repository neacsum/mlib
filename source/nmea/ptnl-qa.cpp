#include "nmeac.h"
/**
  PTNL,QA sentence.

*/
int ptnlqa (const char *buf, double *sigman, double *sigmae, double *smaj,
            double *smin, double *orient)
{
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok, "$PTNL", 5))
    return 0;
  NEXT_TOKEN (tok, 0);
  if (memcmp (tok, "QA", 2))
    return 0;
  NEXT_TOKEN (tok, 0);      //time??
  NEXT_TOKEN (tok, 0);
  IFPAR (sigman, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (sigmae, atof (tok));
  NEXT_TOKEN (tok, 0);      //what?
  NEXT_TOKEN (tok, 0);
  double unit = atof (tok);
  if (sigman) *sigman *= unit;
  if (sigmae) *sigmae *= unit;
  NEXT_TOKEN (tok, 0);
  IFPAR (smaj, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (smin, atof (tok));
  NEXT_TOKEN (tok, 0);
  IFPAR (orient, atof (tok));
  return 1;
}


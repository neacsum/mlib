#include "nmeac.h"

/**
  DBS sentence.

  $ttDBS,depf,f,depm,M,depF,F

  \note
    If more than one depth is specified depth in meters takes precedence
    over depth in feet and depth in feet takes precedence over depth in
    fathoms.

    Returned value is always in meters.
*/
int dbs (const char *buf, double *depth)
{
	double feet = 0.0, meters = 0.0, fathoms = 0.0;
  bool ft, mt, fh;
  ft = mt = fh = false;
  parse_context ctx(buf);

  char *tok = token (ctx);
  if (!tok || memcmp (tok+3, "DBS", 3))
    return 0;
  NEXT_TOKEN (tok, 0);
  if (*tok)
  {
    feet = atof (tok);
    ft = true;
  }
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'f')
    return 0;
  NEXT_TOKEN (tok, 0);
  if (*tok)
  {
    meters = atof (tok);
    mt = true;
  }
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'M')
    return 0;
  NEXT_TOKEN (tok, 0);
  if (*tok)
  {
    fathoms = atof (tok);
    fh = true;
  }
  NEXT_TOKEN (tok, 0);
  if (*tok && *tok != 'F')
    return 0;

  if (depth)
  {
    if (mt) *depth = meters;
    else if (ft) *depth = feet*0.3048;
    else if (fh) *depth = fathoms*1.8288;
   else *depth = 0.;
  }
  return 3;
}


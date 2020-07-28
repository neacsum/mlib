#pragma once
/*!
  \file chull.h Convex hull algorithm

  <pre>

  http://cm.bell-labs.com/who/clarkson/2dch.c

  Ken Clarkson wrote this.  Copyright (c) 1996 by AT&T..
  Permission to use, copy, modify, and distribute this software for any
  purpose without fee is hereby granted, provided that this entire notice
  is included in all copies of any software which is or includes a copy
  or modification of this software and in all copies of the supporting
  documentation for such software.
  THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
  WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
  REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
  OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
  </pre>
*/

#if __has_include ("defs.h")
#include "defs.h"
#endif

#include "point.h"

namespace mlib {

int convex_hull(dpoint *p, int n);

}


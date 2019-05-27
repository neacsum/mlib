#pragma once
/*!
  \file CHULL.H Declaration of convex hull algorithm

  (c) Mircea Neacsu 2017
*/

#if __has_include ("defs.h")
#include "defs.h"
#endif

#include "point.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

int convex_hull(dpoint *p, int n);

#ifdef MLIBSPACE
};
#endif


#pragma once

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


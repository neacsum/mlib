#pragma once

#include <geom/point.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

int convex_hull(dpoint *p, int n);

#ifdef MLIBSPACE
};
#endif


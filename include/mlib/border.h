#pragma once
/*!
  \file border.h Small class to represent simple non-intersecting polygons

  (c) Mircea Neacsu 2017
*/
#if __has_include ("defs.h")
#include "defs.h"
#endif

#include "point.h"

#include <deque>

namespace mlib {


class Border
{
public:

  Border ();
  Border (const char *fname);
  void add (double x, double y);
  void close (double x, double y);
  
  bool inside (double x, double y);

private:
  std::deque <dpoint> vertex;
  dpoint closing;
  int closing_outside;
};

}

#pragma once
/*!
  \file BORDER.H - Small class to represent simple non-intersecting polygons
*/
#include <mlib/defs.h>
#include <geom/point.h>

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

} //namespace

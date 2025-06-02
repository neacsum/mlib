/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

/// \file border.h Small class to represent simple non-intersecting polygons

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include "point.h"

#include <deque>

namespace mlib {

class Border
{
public:
  Border ();
  Border (const char* fname);
  void add (double x, double y);
  void close (double x, double y);

  bool inside (double x, double y);

private:
  std::deque<dpoint> vertex;
  dpoint closing;
  bool closing_outside;
};

} // namespace mlib

#pragma once

/*!
  \file TME.H - Definition of TransverseMercator projection

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Transverse %Mercator projection
class TransverseMercator : public Projection
{
public:
  TransverseMercator () {};
  TransverseMercator (const ProjParams& pp);
  TransverseMercator& operator= (const ProjParams& p);

  errc xy_geo (double x, double y, double *lat, double *lon) const;
  errc geo_xy (double *x, double *y, double lat, double lon) const;
  double k (double lat, double lon) const;
  double h (double lat, double lon) const;

private:
  void init ();
  double n;
  double d0, d2, d4, d6;
  double b0, b2, b4, b6;
  double r;
  double yvalue;
};

inline
double TransverseMercator::h (double lat, double lon) const
{
  return k (lat, lon);
}

#ifdef MLIBSPACE
};
#endif

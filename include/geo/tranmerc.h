#pragma once

/*!
  \file TRANMERC.H - Definition of TransverseMercator projection

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Transverse %Mercator projection
class TransverseMercator : public Projection
{
public:
  TransverseMercator (PROJPARAMS& pp);
  errc XYGeo (double x, double y, double *lat, double *lon) const;
  errc GeoXY (double *x, double *y, double lat, double lon) const;
  double k (double lat, double lon) const;
  const char *Name () const                 { return "TME"; };
  geoproj Id () const                       { return GEOPROJ_TME; };
private:
  double n, e2, ep2;
  double d0, d2, d4, d6;
  double b0, b2, b4, b6;
  double r;
  double yvalue;
};

#ifdef MLIBSPACE
};
#endif

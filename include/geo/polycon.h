#pragma once
/*!
  \file POLYCON.H - Definition of Polyconic projection

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// %Polyconic
class Polyconic : public Projection
{
public:
  Polyconic (PROJPARAMS& pp);
  const char *Name () const     { return "POL"; };
  geoproj Id () const           { return GEOPROJ_POL; };

  errc XYGeo (double x, double y, double *lat, double *lon) const;
  errc GeoXY (double *x, double *y, double lat, double lon) const;
  double h (double lat, double lon) const;

private:
  double m1 (double lat) const;
  double m0;
  double c[4];
};

#ifdef MLIBSPACE
};
#endif

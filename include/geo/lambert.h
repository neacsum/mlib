#pragma once
/*!
  \file LAMBERT.H - \ref Lambert "Lambert Conformal Conical" projection definition

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// %Lambert Conformal Conical
class Lambert : public Projection
{
public:
  Lambert (PROJPARAMS& pp);
  const char *Name() const      { return "LCC"; };
  geoproj Id() const            { return GEOPROJ_LCC; };

  errc XYGeo (double x, double y, double *lat, double *lon) const;
  errc GeoXY (double *x, double *y, double lat, double lon) const;
  double h (double lat, double lon) const {return k(lat, lon);};
  double k (double lat, double lon) const;

private:
  double n, af_big, rho0;
  double tfunc (double phi) const;
};

#ifdef MLIBSPACE
};
#endif

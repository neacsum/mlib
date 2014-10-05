#pragma once

/*!
  \file AZIMEQD.H - \ref AzimuthEqDist "Azimuthal Equidistant" class definition

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Azimuthal Equidistant
class AzimuthEqDist : public Projection
{
public:
  AzimuthEqDist(  PROJPARAMS& pp );
  errc XYGeo(double x, double y, double *lat, double *lon) const;
  errc GeoXY(double *x, double *y, double lat, double lon) const;

  double h (double lat, double lon) const;
  double k (double lat, double lon) const;

  const char *Name() const    { return "AZD"; };
  geoproj Id() const          { return GEOPROJ_AZD; };

private:
  double sphi1, cphi1, n1, g;
  bool north_polar, south_polar;
};


#ifdef MLIBSPACE
};
#endif

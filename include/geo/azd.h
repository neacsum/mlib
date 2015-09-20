#pragma once

/*!
  \file AZD.H - \ref AzimuthEqDist "Azimuthal Equidistant" class definition

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Azimuthal Equidistant
class AzimuthEqDist : public Projection
{
public:
  AzimuthEqDist ();
  AzimuthEqDist (const Params& params);
  AzimuthEqDist& operator= (const Params& p);

  errc xy_geo(double x, double y, double *lat, double *lon) const;
  errc geo_xy(double *x, double *y, double lat, double lon) const;

  double h (double lat, double lon) const;
  double k (double lat, double lon) const;

private:
  void init ();

  double sphi1, cphi1, n1, g;
  bool north_polar, south_polar;
};


#ifdef MLIBSPACE
};
#endif

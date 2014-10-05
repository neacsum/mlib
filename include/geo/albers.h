#pragma once
/*!
  \file ALBERS.H - Definition of \ref Albers "Albers Equal Area" projection

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// %Albers Equal Area
class Albers : public Projection
{
 public:
  Albers (PROJPARAMS& pp);
  const char *Name () const      { return "ALA"; };
  geoproj Id () const            { return GEOPROJ_ALA; };

  errc XYGeo (double x, double y, double *lat, double *lon) const;
  errc GeoXY (double *x, double *y, double lat, double lon) const;

  double h (double lat, double lon) const;
  double k (double lat, double lon) const;

 private:
  double n, c_big, rho0;
};

#ifdef MLIBSPACE
};
#endif


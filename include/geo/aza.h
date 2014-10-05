#pragma once
/*!
  \file AZA.H - Definition of Azimuthal Equal Area projection

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Azimuthal Equal Area
class AZA : public Projection
{
 public:
  AZA (PROJPARAMS& pp);
  const char *Name() const      { return "AZA"; };
  geoproj Id() const            { return GEOPROJ_AZA; };

  errc XYGeo(double x, double y, double *lat, double *lon) const;
  errc GeoXY(double *x, double *y, double lat, double lon) const;

  double h (double lat, double lon) const;
  double k (double lat, double lon) const;

 private:
   double r_q, q_p, q_1, m_1, beta_1, d;
   double ncval;
   bool polar;
};

#ifdef MLIBSPACE
};
#endif

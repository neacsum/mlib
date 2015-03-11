#pragma once
/*!
  \file AZA.H - Definition of Azimuthal Equal Area projection

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Azimuthal Equal Area
class AzimuthalEqualArea : public Projection
{
 public:
  AzimuthalEqualArea ();
  AzimuthalEqualArea (const ProjParams& params);
  AzimuthalEqualArea& operator= (const ProjParams& p);

  errc xy_geo(double x, double y, double *lat, double *lon) const;
  errc geo_xy(double *x, double *y, double lat, double lon) const;

  double h (double lat, double lon) const;
  double k (double lat, double lon) const;

 private:
   void init ();

   double r_q, q_p, q_1, m_1, beta_1, d;
   double ncval;
   bool polar;
};

#ifdef MLIBSPACE
};
#endif
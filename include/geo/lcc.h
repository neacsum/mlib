#pragma once
/*!
  \file LCC.H - \ref Lambert "Lambert Conformal Conical" projection definition

*/

#include "projection.h"

namespace mlib {

/// %Lambert Conformal Conical
class Lambert : public ConicalProjection
{
public:
  Lambert ();
  Lambert (const Params& params);
  Lambert& operator= (const Params& p);

  errc xy_geo (double x, double y, double *lat, double *lon) const;
  errc geo_xy (double *x, double *y, double lat, double lon) const;
  double h (double lat, double lon) const {return k(lat, lon);};
  double k (double lat, double lon) const;


private:
  void init ();
  double n, af_big, rho0;
  double tfunc (double phi) const;
};


} //namespace

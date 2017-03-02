#pragma once
/*!
  \file POLYCON.H - Definition of Polyconic projection

*/

#include "projection.h"

namespace mlib {

/// %Polyconic
class Polyconic : public Projection
{
public:
  Polyconic (const Params& params);
  Polyconic& operator= (const Params& p);

  errc xy_geo (double x, double y, double *lat, double *lon) const;
  errc geo_xy (double *x, double *y, double lat, double lon) const;
  double h (double lat, double lon) const;
  double k (double lat, double lon) const;

private:
  void init ();

  double m1 (double lat) const;
  double m0;
  double c[4];
};

inline
double Polyconic::k (double lat, double lon) const { return 1.; }

} //namespace

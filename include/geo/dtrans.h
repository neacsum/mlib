#pragma once
/*!
  \file DTRANS.H - Definition of datum transformation structure

*/
#include <geo/ellip.h>

namespace mlib {

struct DatumTransformation {
  double dx, dy, dz;
  double drx, dry, drz;
  double k;

  void transform_geo (const Ellipsoid& from, const Ellipsoid& to, double& lat, double& lon, double& h);
  void transform (double& x, double& y, double& z);
};

} //namespace

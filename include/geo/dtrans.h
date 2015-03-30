#pragma once
/*!
  \file DTRANS.H - Definition of datum transformation structure

*/
#include <geo/ellip.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

struct DatumTransformation {
  double dx, dy, dz;
  double drx, dry, drz;
  double k;

  void transform_geo (const Ellipsoid& from, const Ellipsoid& to, double& lat, double& lon, double& h);
  void transform (double& x, double& y, double& z);
};

#ifdef MLIBSPACE
};
#endif

/*!
  \file DTRANS.CPP - datum transformation functions

*/
#include <geo/dtrans.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

void DatumTransformation::transform (double& x, double& y, double& z)
{
  double xi = x, yi = y, zi = z;
  x = dx + (1+k)*(xi + drz*yi - dry*zi);
  y = dy + (1+k)*(-drz*xi + yi + drx*zi);
  z = dz + (1+k)*(dry*xi - drx*yi + zi);
}

void DatumTransformation::transform_geo (const Ellipsoid& from, const Ellipsoid& to, double& lat, double& lon, double& h)
{
  double x, y, z;
  from.geo_ECEF (lat, lon, h, &x, &y, &z);
  transform (x, y, z);
  to.ECEF_geo (&lat, &lon, &h, x, y, z);
}

#ifdef MLIBSPACE
};
#endif

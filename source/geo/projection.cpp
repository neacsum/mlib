/*!
  \file PROJECTI.CPP - Projection class implementation

*/

#include <geo/projection.h>

/*!
  \class Projection
  Derived from Ellipsoid, this class adds functions needed go between
  geographical coordinates and XY coordinates. It is an abstract class
  which serves as a base to different projection systems.
*/

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/*!
 	Create a projection object based on settings from  PROJPARAMS structure.
	Check arguments for legal values.
*/
Projection::Projection (PROJPARAMS& pp):
Ellipsoid (pp.a, (pp.f_1 == 0.)? 0. : 1./pp.f_1),
  k_ (pp.scale),
  central_meridian_ (pp.reflon),
  ref_latitude_ (pp.reflat),
  north_parallel_ (pp.northpar),
  south_parallel_ (pp.southpar),
  unit_ (pp.unit),
  false_east_ (pp.feast),
  false_north_ (pp.fnorth),
  skew_azimuth_ (pp.azskew)
{
  if (k_ == 0.)
    k_ = 1.;
  if (unit_ == 0.)
    unit_ = 1.;
}

/*!
  Default scale factor function. Converts X/Y coordinates to lat/lon and calls
  h and k functions to calculate scale factor along latitude and meridian.
  Returned value is the geometric mean of the the h and k values.
*/
errc Projection::Scale (double x, double y, double *scale)
{
  double lat, lon;
  try {
    XYGeo (x, y, &lat, &lon);
    *scale = sqrt (h(lat, lon)*k(lat, lon));
    return ERR_SUCCESS;
  } catch (errc& ec) {
    return ec;
  }
}

#ifdef MLIBSPACE
};
#endif

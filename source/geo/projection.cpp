/*!
  \file PROJECTION.CPP - Projection class implementation

*/

#include <geo/projection.h>

/*!
  \defgroup geo Geodesy
  \brief Geodesy and map projection classes

  For most projection classes, formulas are taken from

  Snyder, John Parr,
  Map projections--A Working Manual.
  (U.S . Geological Survey professional paper ; 1395)
*/

/*!
  \class Projection
  \ingroup geo
  It is an abstract class which serves as a base to different projection systems.

  The parameters that are used by all or most projection systems (unit, scale factor
  at origin, false easting and northing, etc.) are set using the "named parameter"
  idiom. See: http://www.parashift.com/c++-faq/named-parameter-idiom.html
  
  This leads to eminently readable code like:
\code
  Mercator prj = Projection::Params (Ellipsoid::WGS84)
                   .ref_longitude (45 * D2R)
                   .ref_latitude (49 * D2R)
                   .false_north (1000000);

  prj.geo_xy (&xr, &yr, 49.2166666666*D2R, -123.1*D2R);
\endcode


*/

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

Projection::Params::Params (const Ellipsoid& ell) :
  ellip_ (ell),
  k_ (1.),
  unit_ (1.),
  reflat_ (0.),
  reflon_ (0.),
  fn_ (0.),
  fe_ (0.),
  skew_ (0.)
{
}

Projection::Params::Params (Ellipsoid::well_known wk) :
ellip_ (wk),
k_ (1.),
unit_ (1.),
reflat_ (0.),
reflon_ (0.),
fn_ (0.),
fe_ (0.),
skew_ (0.)
{
}

Projection::Params& Projection::Params::ellipsoid (const Ellipsoid& ell)
{
  ellip_ = ell;
  return *this;
}

Projection::Params& Projection::Params::ellipsoid (Ellipsoid::well_known wk)
{
  ellip_ = Ellipsoid (wk);
  return *this;
}

Projection::Projection ()
{
}

Projection::Projection (const Params& params) :
  par (params)
{
}

/*!
  \class ConicalProjection
  Conical projections have two more parameters, north and south latitude,
  that define the intersection between ellipsoid and projection cone.
*/
ConicalProjection::ConicalProjection ()
{
}

ConicalProjection::ConicalProjection (const Params& params) :
  Projection (params)
{
}

//helper functions
double lon_adjust (double lon)
{
  return ((lon >= 0) ? 1 : -1)*(fmod (fabs (lon) + M_PI, 2 * M_PI) - M_PI);
}

#ifdef MLIBSPACE
};
#endif

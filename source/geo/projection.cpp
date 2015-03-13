/*!
  \file PROJECTI.CPP - Projection class implementation

*/

#include <geo/projection.h>

/*!
  \class Projection
  It is an abstract class which serves as a base to different projection systems.

  The parameters that are used by all or most projection systems (unit, scale factor
  at origin, false easting and northing, etc.) are set using the "named parameter"
  idiom. See: http://www.parashift.com/c++-faq/named-parameter-idiom.html
  
  This leads to eminently readable code like:
\code
  Mercator prj = ProjParams (Ellipsoid::WGS84)
                   .ref_longitude (45 * D2R)
                   .ref_latitude (49 * D2R)
                   .false_north (1000000);

  prj.geo_xy (&xr, &yr, 49.2166666666*D2R, -123.1*D2R);
\endcode


  see also: Curiously Reccuring Template Pattern
  http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern


*/

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

ProjParams::ProjParams (const Ellipsoid& ell) :
  ellip_ (ell),
  k_ (1.),
  unit_ (1.),
  reflat_ (0.),
  reflon_ (0.),
  fn_ (0.),
  fe_ (0.),
  npar_ (0.),
  spar_ (0.),
  skew_ (0.)
{
}

ProjParams::ProjParams (Ellipsoid::well_known wk) :
ellip_ (wk),
k_ (1.),
unit_ (1.),
reflat_ (0.),
reflon_ (0.),
fn_ (0.),
fe_ (0.),
npar_ (0.),
spar_ (0.),
skew_ (0.)
{
}

ProjParams& ProjParams::ellipsoid (const Ellipsoid& ell)
{
  ellip_ = ell;
  return *this;
}

ProjParams& ProjParams::ellipsoid (Ellipsoid::well_known wk)
{
  ellip_ = Ellipsoid (wk);
  return *this;
}

Projection::Projection ()
{
}

Projection::Projection (const ProjParams& params) :
  par (params)
{
}

ConicalProjection::ConicalProjection ()
{
}

ConicalProjection::ConicalProjection (const ProjParams& params) :
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

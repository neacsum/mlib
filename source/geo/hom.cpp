/*!
  \file HOM.CPP - \ref Hotine "Hotine Oblique Mercator" implementation

*/

#include <geo/hom.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/*! Rotation from uv coordinate system to XY. Also performs
		translation to false_east/false_north origin and unit change.
		(the unit for uv system is meters).
*/
void Hotine::deskew( double u, double v, double *x, double *y ) const
{
  double sskew = sin (skew_azimuth_);
  double cskew = cos (skew_azimuth_);
  *x = (cskew * v + sskew * u)/unit_ + false_east_;
  *y = (-sskew * v + cskew * u)/unit_ + false_north_;
}

/*!
  Rotation form xy coordinate system to uv
*/
void Hotine::skew( double *u, double *v, double x, double y ) const
{
  double sskew = sin (skew_azimuth_);
  double cskew = cos (skew_azimuth_);
  x -= false_east_;
  y -= false_north_;
  x *=unit_;
  y *= unit_;
  *v = cskew * x - sskew * y;
  *u = sskew * x + cskew * y;
}

#ifdef MLIBSPACE
};
#endif

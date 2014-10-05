/*!
  \file RSO.CPP - Rectified Skew Orthomorphic implementation

*/

#include <geo/rso.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/*!
  Rotation from uv coordinate system to XY. Also performs
  translation to false_east/false_north origin and unit change.
  (the unit for uv system is meters).
*/
void RSO::deskew (double u, double v, double *x, double *y) const
{
  double sskew = sin (gama0);
  double cskew = cos (gama0);
  *x = (cskew * v + sskew * u) / unit_ + false_east_;
  *y = (-sskew * v + cskew * u) / unit_ + false_north_;
}

/*!
  Rotation form xy coordinate system to uv
*/
void RSO::skew (double *u, double *v, double x, double y) const
{
  double sskew = sin (gama0);
  double cskew = cos (gama0);
  x -= false_east_;
  y -= false_north_;
  x *= unit_;
  y *= unit_;
  *v = cskew * x - sskew * y;
  *u = sskew * x + cskew * y;
}

#ifdef MLIBSPACE
};
#endif

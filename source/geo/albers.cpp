/*  \file ALBERS.CPP - Implementation of Albers Equal Area projection

      Formulas from Snyder pag. 101-102
*/

#include <geo/albers.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

#define MAX_ITER  10    //max number of iterations in reverse formulas

/*!
  Constructor for a Albers Equal Area projection
*/
Albers::Albers (PROJPARAMS& pp):
  Projection (pp)
{
  double m1 = m(north_parallel_);
  double q1 = q(north_parallel_);
  if (north_parallel_ == south_parallel_)
    n = sin(north_parallel_);
  else
  {
    double m2 = m(south_parallel_);
    double q2 = q(south_parallel_);
    n = (m1*m1 - m2*m2) / (q2-q1);
  }
  c_big = m1*m1+n*q1;
  rho0 = a() / unit_ * sqrt(c_big - n * q(ref_latitude_)) / n;
  if ( n == 0 )
    throw errc (GEOERR_PARM);
}

/*!
  Forward conversion from latitude/longitude to XY
*/
errc Albers::GeoXY (double *x, double *y, double lat, double lon) const
{
  double theta = (lon - central_meridian_) * n;
  double rho = a() / unit_ * sqrt(c_big - n * q(lat)) / n;
  *x = false_east_ + rho * sin(theta);
  *y = false_north_ + rho0 - rho * cos(theta);
  return ERR_SUCCESS;
}

/*!
  Inverse conversion from XY to geographical coordinates.
*/
errc Albers::XYGeo (double x, double y, double *lat, double *lon) const
{
  x -= false_east_;
  y -= false_north_;
  double yprime = rho0-y;
  double rho = hypot (x,yprime);
  double theta = atan (x/yprime);
  *lon = theta/n + central_meridian_;

  double q = (c_big-rho*rho*n*n/(a()*a()/(unit_*unit_)))/n;
  if (fabs(q) > 2)
      return errc (GEOERR_DOMAIN);

  double phi = asin (q/2);
  int iter =0;
  double delta = 10;
  while (fabs(delta) > 1e-7 && iter++ < 10)
  {
    double sinphi = sin (phi);
    double v1 = 1 - e2 ()*sinphi*sinphi;
    delta = v1*v1 / (2 * cos (phi))*(q / (1 - e2 ()) - sinphi / v1 + t (phi));
    phi += delta;
  }
  *lat = phi;
  if (iter >= MAX_ITER)
    return errc (GEOERR_NCONV);

  return ERR_SUCCESS;
}

double Albers::h (double lat, double lon) const
{
  return sqrt(c_big - n * q(lat))/m(lat);
}

double Albers::k (double lat, double lon) const
{
  if (fabs(lat) != M_PI_2)
    return 1/h(lat, lon);
  else
    throw errc(GEOERR_DOMAIN);
}

#ifdef MLIBSPACE
};
#endif

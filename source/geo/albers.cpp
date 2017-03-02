/*  \file ALBERS.CPP - Implementation of Albers Equal Area projection

      Formulas from Snyder pag. 101-102
*/

#include <geo/albers.h>

static double sqrarg;
#define SQR(a) ((sqrarg = (a)) == 0.0 ? 0.0 : sqrarg * sqrarg)

namespace mlib {

#define MAX_ITER  10    //max number of iterations in reverse formulas

Albers::Albers ()
{
}


/*!
  Constructor for a Albers Equal Area projection
*/
Albers::Albers (const Params& params):
  ConicalProjection (params)
{
  init ();
}

Albers& Albers::operator= (const Params& p)
{
  par = p;
  init ();
  return *this;
}

void Albers::init ()
{
  double m1 = ellipsoid ().m (north_latitude ());
  double q1 = ellipsoid ().q (north_latitude ());
  if (north_latitude () == south_latitude ())
    n = sin (north_latitude ());
  else
  {
    double m2 = ellipsoid ().m (south_latitude ());
    double q2 = ellipsoid ().q (south_latitude ());
    n = (m1*m1 - m2*m2) / (q2 - q1);
  }
  c_big = m1*m1 + n*q1;
  rho0 = ellipsoid ().a () / unit () * sqrt (c_big - n * ellipsoid ().q (ref_latitude ())) / n;
  if (n == 0)
    throw errc (GEOERR_PARAM);
}

/*!
  Forward conversion from latitude/longitude to XY
*/
errc Albers::geo_xy (double *x, double *y, double lat, double lon) const
{
  double theta = (lon - ref_longitude()) * n;
  double rho = ellipsoid().a() / unit() * sqrt(c_big - n * ellipsoid().q(lat)) / n;
  *x = false_east() + rho * sin(theta);
  *y = false_north() + rho0 - rho * cos(theta);
  return ERR_SUCCESS;
}

/*!
  Inverse conversion from XY to geographical coordinates.
*/
errc Albers::xy_geo (double x, double y, double *lat, double *lon) const
{
  x -= false_east();
  y -= false_north();
  double yprime = rho0-y;
  double rho = hypot (x,yprime);
  double theta = atan (x/yprime);
  *lon = theta/n + ref_longitude();

  double q = (c_big-SQR(rho*n*unit()/ellipsoid().a()))/n;
  if (fabs(q) > 2)
      return errc (GEOERR_DOMAIN);

  double phi = asin (q/2);
  int iter =0;
  double delta = 10;
  while (fabs(delta) > 1e-7 && iter++ < 10)
  {
    double sinphi = sin (phi);
    double v1 = 1 - ellipsoid().e2 ()*sinphi*sinphi;
    delta = v1*v1 / (2 * cos (phi))*(q / (1 - ellipsoid().e2 ()) - sinphi / v1 + ellipsoid().t (phi));
    phi += delta;
  }
  *lat = phi;
  if (iter >= MAX_ITER)
    return errc (GEOERR_NCONV);

  return ERR_SUCCESS;
}

double Albers::k (double lat, double lon) const
{
  return sqrt(c_big - n * ellipsoid().q(lat))/ellipsoid().m(lat);
}

double Albers::h (double lat, double lon) const
{
  if (fabs(lat) != M_PI_2)
    return 1/k(lat, lon);
  else
    throw errc(GEOERR_SINGL);
}

} //namespace

/*!
  \file LCC.CPP - \ref Lambert "Lambert Conformal Conical" implementation

*/

/*!
  \class Lambert
  Formulas from Snyder pag. 107-109
*/
#include <geo/lcc.h>

static double sqrarg;
#define SQR(a) ((sqrarg = (a)) == 0.0 ? 0.0 : sqrarg * sqrarg)

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

#define MAX_ITER  10    //max number of iterations in reverse formulas

/*!
  Constructor for a %Lambert Conformal Conical projection
*/
Lambert::Lambert (const ProjParams& params):
  ConicalProjection (params)
{
  init ();
}

Lambert& Lambert::operator= (const ProjParams& p)
{
  par = p;
  init ();
  return *this;
}

void Lambert::init()
{
  double m1 = cos(north_latitude()) / sqrt(1 - ellipsoid().e2()*SQR(sin(north_latitude())));
  double t1 = tfunc(north_latitude());
  if (north_latitude()== south_latitude())
    n = sin(north_latitude());
  else
  {
    double m2 = cos(south_latitude()) / sqrt(1 - ellipsoid().e2()*SQR(sin(south_latitude())));
    n = log(m1 / m2) / log(t1 / tfunc(south_latitude()));
  }
  af_big = ellipsoid().a() / unit() * k0() * m1 / (n*pow(t1, n));
  rho0 = af_big * pow(tfunc(ref_latitude()), n);
  if (n == 0)
    throw errc(GEOERR_PARAM);
}

/*!
  Forward conversion from latitude/longitude to XY
*/
errc Lambert::geo_xy (double *x, double *y, double lat, double lon) const
{
  double theta = (lon - ref_longitude()) * n;
  double rho;
  if (fabs(lat) == M_PI_2)
  {
    if (n*lat > 0.)
      rho = 0;
    else
      return errc (GEOERR_SINGL);
  }
  else
    rho = af_big * pow(tfunc(lat), n);
  *x = false_east() + rho * sin(theta);
  *y = false_north() + rho0 - rho * cos(theta);
  return ERR_SUCCESS;
}

/*!
  Inverse conversion from XY to geographical coordinates.
*/
errc Lambert::xy_geo (double x, double y, double *lat, double *lon) const
{
  x -= false_east ();
  y -= false_north();
  double yprime = rho0-y;
  double rho = hypot (x,yprime);
  double theta;
  if (n < 0.)
  {
    rho *=-1;
    theta = atan(-x/(y-rho0));
  }
  else
    theta = atan(x/yprime);
  *lon = theta/n + ref_longitude();
  double t = pow( rho/af_big, 1/n);

  double e_ = ellipsoid().e();
  double phi_right = M_PI_2 - 2*atan(t);
  int iter =0;
  double delta = 10;
  while (delta > 1e-10 && iter++ < 10)
  {
    double sinphi = sin(phi_right);
    double phi_left = M_PI_2 - 2*atan(t*pow((1-e_*sinphi)/(1+e_*sinphi),e_/2));
    delta = fabs(phi_left-phi_right);
    phi_right = phi_left;
  }
  *lat = phi_right;
  if (iter >= MAX_ITER)
    return errc (GEOERR_NCONV);

  return ERR_SUCCESS;
}

double Lambert::k (double lat, double lon) const
{
  double rho = af_big * pow(tfunc(lat), n);
  return rho*n/(ellipsoid().a()*unit()*ellipsoid().m(lat));
}

/// helper function. Not to be confused with the Ellipsoid::t function
double Lambert::tfunc (double phi) const
{
  double sval = sin(phi);
  double t1 = (1. - sval) / (1. + sval);
  double e_ = ellipsoid().e();
  double t2 = pow((1. + e_ * sval) / (1. - e_ * sval), e_);
  return (sqrt(t1 * t2));
}

#ifdef MLIBSPACE
};
#endif


/*!
  \file OCY.CPP - Oblique Cylindrical implementation

*/

#include <geo/ocy.h>
#include <geo/convert.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

ObliqueCylindrical::ObliqueCylindrical (const Params& params) :
  Projection (params)
{
  init ();
}

ObliqueCylindrical& ObliqueCylindrical::operator= (const Params& p)
{
  par = p;
  init ();
  return *this;
}

void ObliqueCylindrical::init ()
{
  double e2 = ellipsoid ().e2 ();
  c1 = sqrt (1 + e2 / (1 - e2)*pow (cos (ref_latitude()), 4));
  chi0 = asin (sin (ref_latitude()) / c1);
  double b0prim = asin (ellipsoid().e ()*sin (ref_latitude()));
  c2 = log (tan (M_PI_4 + chi0 / 2.)) - c1*log (tan (M_PI_4 + ref_latitude() / 2.)) +
    c1 * ellipsoid().e () * log (tan (M_PI_4 + b0prim / 2));
  double x = sqrt (1 - e2 *sin (ref_latitude ())*sin (ref_latitude ()));
  double n0 = ellipsoid().a () / x;
  double m0 = ellipsoid().a ()*(1 - e2) / pow (x, 3);
  map_radius = k0 ()*sqrt (n0*m0) / unit();
}

errc ObliqueCylindrical::geo_xy (double *x, double *y, double lat, double lon) const
{
  double l = c1 * (lon - ref_longitude());
  double bprim = asin (ellipsoid().e () * sin (lat));
  double tg = exp (c1*log (tan (M_PI_4 + lat / 2)) - c1*ellipsoid().e ()*log (tan (M_PI_4 + bprim / 2)) + c2);
  double b = 2 * atan (tg) - M_PI_2;

  double bbar = asin (cos (chi0)*sin (b) - sin (chi0)*cos (b)*cos (l));
  double lbar = asin (cos (b)*sin (l) / cos (bbar));
  *x = map_radius*lbar + false_east();
  *y = map_radius*log (tan (M_PI_4 + bbar / 2)) + false_north();
  return ERR_SUCCESS;
}

errc ObliqueCylindrical::xy_geo (double x, double y, double *lat, double *lon) const
{
  const int MAX_ITER = 30;
  x -= false_east();
  y -= false_north();

  double xbar = exp (y / map_radius);
  double lbar = x / map_radius;
  double bbar = 2 * atan (xbar) - M_PI_2;
  double b = asin (cos (chi0)*sin (bbar) + sin (chi0)*cos (bbar)*cos (lbar));
  double l = asin (cos (bbar)*sin (lbar) / cos (b));

  double phi = ref_latitude();
  double delta = 100.;
  double temp1 = (log (tan (M_PI_4 + b / 2)) - c2) / c1;
  int i = 0;
  while (delta > 0.1*MAS && i<MAX_ITER)
  {
    double bprim = asin (ellipsoid().e ()*sin (phi));
    double tg = exp (temp1 + ellipsoid().e ()*log (tan (M_PI_4 + bprim / 2)));
    double phi_new = 2 * atan (tg) - M_PI_2;
    delta = fabs (phi - phi_new);
    phi = phi_new;
    i++;
  }
  if (i == MAX_ITER)
    return GEOERR_NCONV;

  *lat = phi;
  *lon = ref_longitude() + l / c1;
  return ERR_SUCCESS;
}

double ObliqueCylindrical::k (double lat, double lon) const
{
  double l = c1 * (lon - ref_longitude());
  double bprim = asin (ellipsoid().e () * sin (lat));
  double tg = exp (c1*log (tan (M_PI_4 + lat / 2)) - c1*ellipsoid().e ()*log (tan (M_PI_4 + bprim / 2)) + c2);
  double b = 2 * atan (tg) - M_PI_2;

  double bbar = asin (cos (chi0)*sin (b) - sin (chi0)*cos (b)*cos (l));

  double kval = c1*map_radius / ellipsoid().rn (lat)*cos (b) / (cos (lat)*cos (bbar));
  return kval;
}

#ifdef MLIBSPACE
};
#endif

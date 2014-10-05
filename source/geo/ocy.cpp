/*!
  \file OCY.CPP - Oblique Cylindrical implementation

*/

#include <geo/ocy.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

ObliqueCylindrical::ObliqueCylindrical (PROJPARAMS& pp) :
  Projection (pp)
{
  c1 = sqrt (1 + e2 () / (1 - e2 ())*pow (cos (ref_latitude_), 4));
  chi0 = asin (sin (ref_latitude_) / c1);
  double b0prim = asin (e ()*sin (ref_latitude_));
  c2 = log (tan (M_PI_4 + chi0 / 2.)) - c1*log (tan (M_PI_4 + ref_latitude_ / 2.)) +
    c1 * e () * log (tan (M_PI_4 + b0prim / 2));
  double n0 = a () / sqrt (1 - e2 ()*sin (ref_latitude_)*sin (ref_latitude_));
  double m0 = a ()*(1 - e2 ()) / pow (1 - e2 ()*sin (ref_latitude_)*sin (ref_latitude_), 1.5);
  map_radius = ScaleFactor ()*sqrt (n0*m0) / unit_;
}

errc ObliqueCylindrical::GeoXY (double *x, double *y, double lat, double lon) const
{
  double l = c1 * (lon - central_meridian_);
  double bprim = asin (e () * sin (lat));
  double tg = exp (c1*log (tan (M_PI_4 + lat / 2)) - c1*e ()*log (tan (M_PI_4 + bprim / 2)) + c2);
  double b = 2 * atan (tg) - M_PI_2;

  double bbar = asin (cos (chi0)*sin (b) - sin (chi0)*cos (b)*cos (l));
  double lbar = asin (cos (b)*sin (l) / cos (bbar));
  *x = map_radius*lbar + false_east_;
  *y = map_radius*log (tan (M_PI_4 + bbar / 2)) + false_north_;
  return ERR_SUCCESS;
}

errc ObliqueCylindrical::XYGeo (double x, double y, double *lat, double *lon) const
{
  x -= false_east_;
  y -= false_north_;

  double xbar = exp (y / map_radius);
  double lbar = x / map_radius;
  double bbar = 2 * atan (xbar) - M_PI_2;
  double b = asin (cos (chi0)*sin (bbar) + sin (chi0)*cos (bbar)*cos (lbar));
  double l = asin (cos (bbar)*sin (lbar) / cos (b));

  double phi = ref_latitude_;
  double delta = 100.;
  double temp1 = (log (tan (M_PI_4 + b / 2)) - c2) / c1;
  while (delta > 1e-7)
  {
    double bprim = asin (e ()*sin (phi));
    double tg = exp (temp1 + e ()*log (tan (M_PI_4 + bprim / 2)));
    double phi_new = 2 * atan (tg) - M_PI_2;
    delta = fabs (phi - phi_new);
    phi = phi_new;
  }
  *lat = phi;
  *lon = central_meridian_ + l / c1;
  return ERR_SUCCESS;
}

double ObliqueCylindrical::k (double lat, double lon) const
{
  double l = c1 * (lon - central_meridian_);
  double bprim = asin (e () * sin (lat));
  double tg = exp (c1*log (tan (M_PI_4 + lat / 2)) - c1*e ()*log (tan (M_PI_4 + bprim / 2)) + c2);
  double b = 2 * atan (tg) - M_PI_2;

  double bbar = asin (cos (chi0)*sin (b) - sin (chi0)*cos (b)*cos (l));

  double kval = c1*rn (ref_latitude_) / rn (lat)*cos (b) / (cos (lat)*cos (bbar));
  return kval;
}

#ifdef MLIBSPACE
};
#endif

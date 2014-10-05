/*!
  \file CASSINI.CPP - Cassini projection implementation

*/

/*!
  \class Cassini
   Formulas from Snyder pag. 95
*/
#include <geo/cassini.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

Cassini::Cassini (PROJPARAMS& pp):
  Projection (pp),
  s0 (lm(ref_latitude_)/unit_)
{
}

errc Cassini::GeoXY (double *x, double *y, double lat, double lon) const
{
  double a, c, t, tmp;
  double r = rn(lat)/unit_;
  a = (lon-central_meridian_)*cos(lat);
  c = ep2()*(tmp=cos(lat))*tmp;
  t = (tmp=tan(lat))*tmp;

  *x = r*(a - t*a*a*a/6. - (8 - t + 8*c)*t*(tmp=a*a)*tmp*a/120.) + false_east_;
  *y = lm(lat)/unit_ - s0 + r*tan(lat)*(a*a/2. + (5 - t + 6*c)*(tmp=a*a)*tmp/24.) + false_north_;
  return ERR_SUCCESS;
}

errc Cassini::XYGeo (double x, double y, double *lat, double *lon) const
{
  double s1, mu, eps, phi1, d, t, t2, tmp;
  
  s1 = (s0 + y-false_north_)*unit_;
  mu = s1/(a()*(1-e2()/4.-3*e2()*e2()/64.-5.*e2()*e2()*e2()/256.));
  eps = (a()-b())/(a()+b());
  
  tmp=eps*eps;
  phi1 = mu 
    + (3./2. - 27*eps*eps/32.)*eps*sin(2*mu)
    + tmp*(21./16. - 55*tmp/32.)*sin(4*mu)
    + 1097.*tmp*tmp/512.*sin(8*mu);

  d = (x-false_east_)*unit_/rn(phi1);
  t = tan(phi1);
  t2 = t*t;

  tmp=d*d;
  *lat = phi1 - rn(phi1)*t/rm(phi1)*(tmp/2.-(1.-3.*t2)*tmp*tmp/24.);
  *lon = central_meridian_ + 1/cos(phi1)*(d-t2*(tmp=d*d*d)/3+(1+3*t2)*tmp*d*d/15);
  return ERR_SUCCESS;
}

double Cassini::h (double lat, double lon) const
{
  return 1/sqrt(1-ipow(cos(lat)*sin(lon-central_meridian_), 2));
}

#ifdef MLIBSPACE
};
#endif

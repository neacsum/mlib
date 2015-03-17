/*!
  \file CAS.CPP - Cassini projection implementation

*/

/*!
  \class Cassini
   Formulas from Snyder pag. 95
*/
#include <geo/cas.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

Cassini::Cassini (const ProjParams& params):
  Projection (params)
{
  init ();
}

Cassini& Cassini::operator= (const ProjParams& p)
{
  par = p;
  init ();
  return *this;
}

void Cassini::init ()
{
  s0 = ellipsoid ().lm (ref_latitude ()) / unit ();
}

errc Cassini::geo_xy (double *x, double *y, double lat, double lon) const
{
  double a, c, t, tmp;
  double r = ellipsoid().rn(lat)/unit();
  a = (lon-ref_longitude())*cos(lat);
  c = ellipsoid().ep2()*(tmp=cos(lat))*tmp;
  t = (tmp=tan(lat))*tmp;

  *x = r*(a - t*a*a*a/6. - (8 - t + 8*c)*t*(tmp=a*a)*tmp*a/120.) + false_east();
  *y = ellipsoid().lm(lat)/unit() - s0 + r*tan(lat)*(a*a/2. + (5 - t + 6*c)*(tmp=a*a)*tmp/24.) + false_north();
  return ERR_SUCCESS;
}

errc Cassini::xy_geo (double x, double y, double *lat, double *lon) const
{
  double s1, mu, eps, phi1, d, t, t2, tmp;
  
  s1 = (s0 + y-false_north())*unit();
  double e2 = ellipsoid ().e2 ();
  mu = s1/(ellipsoid().a()*(1-e2/4.-3*e2*e2/64.-5.*e2*e2*e2/256.));
  eps = (ellipsoid().a()-ellipsoid().b())/(ellipsoid().a()+ellipsoid().b());
  
  tmp=eps*eps;
  phi1 = mu 
    + (3./2. - 27*eps*eps/32.)*eps*sin(2*mu)
    + tmp*(21./16. - 55*tmp/32.)*sin(4*mu)
    + 1097.*tmp*tmp/512.*sin(8*mu);

  d = (x-false_east())*unit()/ellipsoid().rn(phi1);
  t = tan(phi1);
  t2 = t*t;

  tmp=d*d;
  *lat = phi1 - ellipsoid().rn(phi1)*t/ellipsoid().rm(phi1)*(tmp/2.-(1.-3.*t2)*tmp*tmp/24.);
  *lon = ref_longitude() + 1/cos(phi1)*(d-t2*(tmp=d*d*d)/3+(1+3*t2)*tmp*d*d/15);
  return ERR_SUCCESS;
}

double Cassini::h (double lat, double lon) const
{
  return 1/sqrt(1-pow(cos(lat)*sin(lon-ref_longitude()), 2));
}

#ifdef MLIBSPACE
};
#endif

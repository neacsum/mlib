/*!
  \file OME.CPP - ObliqueMercator implementation

*/

#include <geo/ome.h>

namespace mlib {

/*!
  \class ObliqueMercator
  There are different variants for this projection system. In one variant, that
  we call "Rectified Skew Orthomorphic", the rectified coordinate system is 
  defined so that grid north coincides with true north at the natural origin of
  the projection (u=0, v=0). The other variant, that we call "%Hotine", the grid
  north coincides with true north at the center of the projection (ref_latitude,
  ref_longitude). The only difference is angle applied for grid rotation. 
  In the first case the angle is "gamma0" while in the second case is the
  azimuth of the initial line (skew_azimuth). 

  Each variant is implemented as a derived class with only skewing and de-skweing
  function different between the two.
*/

ObliqueMercator::ObliqueMercator (const Params& params) :
  Projection (params)
{
  init ();
}

ObliqueMercator& ObliqueMercator::operator= (const Params& p)
{
  par = p;
  init ();
  return *this;
}

void ObliqueMercator::init ()
{
  B = sqrt (1 + ellipsoid().e2 ()*ipow (cos (ref_latitude()), 4) / (1 - ellipsoid().e2 ()));
  double v1 = sqrt (1 - ellipsoid().e2 ());
  double v2 = 1 - ellipsoid().e2 ()*sin (ref_latitude())*sin (ref_latitude());
  A = ellipsoid().a ()* B * k0() * v1 / v2;
  double D = B * v1 / (cos (ref_latitude())*sqrt (v2));
  double F = D + ((ref_latitude() >= 0) ? sqrt (D*D - 1) : -sqrt (D*D - 1));
  double t0 = exptau (ref_latitude());
  E = F * pow (t0, B);
  double G = (F - 1. / F) / 2;
  gamma0 = asin (sin (skew_azimuth()) / D);
  double delta = asin (G*tan (gamma0)) / B;
  lam1 = ref_longitude() - asin (G*tan (gamma0)) / B;
}

errc ObliqueMercator::geo_xy( double *x, double *y, double lat, double lon ) const
{
  double Q = E/pow( exptau(lat), B );
  double S = (Q-1./Q)/2.;
  double T = (Q+1./Q)/2.;
  double V = sin( B*(lon-lam1) );
  double U = (-V*cos(gamma0) + S*sin(gamma0))/T;
  double v = A*log((1.-U)/(1.+U))/(2.*B);
  double u = A/B*atan2(S*cos(gamma0)+V*sin(gamma0),cos(B*(lon-lam1)));
  deskew( u, v, x, y );
  return ERR_SUCCESS;
}

double ObliqueMercator::k(double lat, double lon) const
{
  double scale;
  double Q = E/pow( exptau(lat), B );
  double S = (Q-1./Q)/2.;
  double T = (Q+1./Q)/2.;
  double V = sin( B*(lon-lam1) );
  double U = (-V*cos(gamma0) + S*sin(gamma0))/T;
  double u = A/B*atan2(S*cos(gamma0)+V*sin(gamma0),cos(B*(lon-lam1)));
  scale = A*cos (B*u / A)*sqrt (1 - ellipsoid ().e2 ()*sin (lat)*sin (lat)) / 
    (ellipsoid ().a ()*cos (lat)*cos (B*(lon - lam1)));
  return scale;
}

errc ObliqueMercator::xy_geo(double x, double y, double *lat, double *lon ) const
{
  double u, v;
  skew( &u, &v, x, y );
  return uvgeo (u, v, lat, lon);
}

/// helper function
double ObliqueMercator::exptau(double value) const
{
  value = sin(value);
  double t1 = (1. - value) / (1. + value);
  double e_val = ellipsoid().e();
  double t2 = pow((1. + e_val * value) / (1. - e_val * value), e_val);

  double result = sqrt(t1*t2);
  return result;
}


/// Conversion from uv (skewed coords) to geographic
errc ObliqueMercator::uvgeo(double u, double v, double *lat, double *lon) const
{
  double Q = exp( -B/A*v );
  double S = (Q-1./Q)/2.;
  double T = (Q+1./Q)/2.;
  double V = sin( B*u/A );
  double U = (V*cos(gamma0)+S*sin(gamma0))/T;
  double t = pow( E/sqrt((1.+U)/(1.-U)), 1./B );
  double ll = M_PI/2 - 2*atan(t);
  int i=0;
  double err = 1.;
  double e_= ellipsoid().e();
  while( err > 1e-9 && i<30 )
  {
    double ll1 = M_PI/2-2*atan(t*pow((1-e_*sin(ll))/(1+e_*sin(ll)), e_/2.));
    err = fabs(ll-ll1);
    ll = ll1;
    i++;
  }
  if (err > 1e-9)
    return GEOERR_NCONV;

  *lat = ll;
  *lon = lam1-atan((S*cos(gamma0)-V*sin(gamma0))/cos(B*u/A))/B;
  return ERR_SUCCESS;
}


/*!
  Rotation from uv coordinate system to XY. Also performs
  translation to false_east/false_north origin and unit change.
  (the unit for uv system is meters).

  For this variant the rotation angle is the same as the azimuth of the
  initial line (skew_azimuth).
*/
void Hotine::deskew (double u, double v, double *x, double *y) const
{
  double sskew = sin (skew_azimuth ());
  double cskew = cos (skew_azimuth ());
  *x = (cskew * v + sskew * u) / unit () + false_east ();
  *y = (-sskew * v + cskew * u) / unit () + false_north ();
}

/*!
  Rotation form xy coordinate system to uv
*/
void Hotine::skew (double *u, double *v, double x, double y) const
{
  double sskew = sin (skew_azimuth ());
  double cskew = cos (skew_azimuth ());
  x -= false_east ();
  y -= false_north ();
  x *= unit ();
  y *= unit ();
  *v = cskew * x - sskew * y;
  *u = sskew * x + cskew * y;
}

/*!
  Rotation from uv coordinate system to XY. Also performs
  translation to false_east/false_north origin and unit change.
  (the unit for uv system is meters).

  For this variant the rotation angle is "gamma0", the angle between the
  initial line and the meridian at the origin of the (u,v) coordinate system
  (the natural origin).
*/
void RSO::deskew (double u, double v, double *x, double *y) const
{
  double sskew = sin (gamma0);
  double cskew = cos (gamma0);
  *x = (cskew * v + sskew * u) / unit () + false_east ();
  *y = (-sskew * v + cskew * u) / unit () + false_north ();
}

/*!
  Rotation form xy coordinate system to uv
*/
void RSO::skew (double *u, double *v, double x, double y) const
{
  double sskew = sin (gamma0);
  double cskew = cos (gamma0);
  x -= false_east ();
  y -= false_north ();
  x *= unit ();
  y *= unit ();
  *v = cskew * x - sskew * y;
  *u = sskew * x + cskew * y;
}


} //namespace

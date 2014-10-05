/*!
  \file OME.CPP - ObliqueMercator implementation

*/

#include <geo/ome.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

ObliqueMercator::ObliqueMercator (PROJPARAMS& pp) :
  Projection( pp )
{
  B = sqrt( 1 + e2()*pow( cos(ref_latitude_), 4)/(1-e2()) );
  double v1 = sqrt( 1-e2() );
  double v2 = 1-e2()*sin(ref_latitude_)*sin(ref_latitude_);
  A = a()* B * k_ * v1/v2;
  double D = B * v1 / (cos(ref_latitude_)*sqrt(v2));
  double F = D + ((ref_latitude_>=0)?sqrt( D*D-1 ) : -sqrt(D*D-1));
  double t0= exptau(ref_latitude_);
  E = F * pow( t0, B );
  double G = (F - 1./F)/2;
  gama0 = asin (sin(skew_azimuth_)/D);
  lam1 = central_meridian_ - asin( G*tan(gama0) )/B;
}

errc ObliqueMercator::GeoXY( double *x, double *y, double lat, double lon ) const
{
  double Q = E/pow( exptau(lat), B );
  double S = (Q-1./Q)/2.;
  double T = (Q+1./Q)/2.;
  double V = sin( B*(lon-lam1) );
  double U = (-V*cos(gama0) + S*sin(gama0))/T;
  double v = A*log((1.-U)/(1.+U))/(2.*B);
  double u = A/B*atan2(S*cos(gama0)+V*sin(gama0),cos(B*(lon-lam1)));
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
  double U = (-V*cos(gama0) + S*sin(gama0))/T;
  double v = A*log((1.-U)/(1.+U))/(2.*B);
  double u = A/B*atan2(S*cos(gama0)+V*sin(gama0),cos(B*(lon-lam1)));
  scale = A*cos(B*u/A)*sqrt(1-e2()*sin(lat)*sin(lat))/a()*cos(lat)*cos(B*(lon-central_meridian_));
  return scale;
}

errc ObliqueMercator::XYGeo(double x, double y, double *lat, double *lon ) const
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
  double e_val = e();						// don't calculate it every time
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
  double U = (V*cos(gama0)+S*sin(gama0))/T;
  double t = pow( E/sqrt((1.+U)/(1.-U)), 1./B );
  double ll = M_PI/2 - 2*atan(t);
  int i=0;
  double err = 1.;
  double e_= e();
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
  *lon = lam1-atan((S*cos(gama0)-V*sin(gama0))/cos(B*u/A))/B;
  return ERR_SUCCESS;
}

#ifdef MLIBSPACE
};
#endif

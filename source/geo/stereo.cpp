/*!
  \file STEREO.CPP - Implementation of Sterographic and Polar Stereograpic projections  

*/

#include <geo/stereo.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/*!
  Ctor for Sterographic projection. 
  Formulas from:<blockquote>
    A manual for geodetic coordinate transformations in the maritime provinces, 
    D.B. Thomson, E.J. Krakiwsky, R.R. Steeves, Technical Report No. 48, July 
    1977, Department of Geodesy and Geomatics Engineering, University of New 
    Brunswick, Canada.</blockquote>

  Calculate parameters which are constant for this projection:
  - c1, c2 - constants used in transformation from ellipsoid to conformal sphere
  - chi0, lam0s - spherical lat/lon of origin of projection.
  - map_radius - radius of the projection sphere
  - r0 -
*/
Stereographic::Stereographic (const Params& params) :
  Projection (params)
{
  init ();
}

void Stereographic::init ()
{
  double e2 = ellipsoid ().e2 ();
  double sphi0 = sin (ref_latitude ());
  c1 = sqrt (1 + e2 / (1 - e2 )*pow (cos (ref_latitude()), 4));
  chi0 = asin (sphi0 / c1);
  lam0s = ref_longitude()*c1;
  c2 = tan (M_PI_4 + chi0 / 2.) / pow (tan (M_PI_4 + ref_latitude() / 2.)*
    pow ((1 - ellipsoid().e ()*sphi0) / (1 + ellipsoid().e ()*sphi0), ellipsoid().e () / 2.), c1);
  double n0 = ellipsoid().a () / sqrt (1 - e2 *sphi0*sphi0);
  double m0 = ellipsoid().a ()*(1 - e2) / pow (1 - e2 *sphi0*sphi0, 1.5);

  map_radius = sqrt (n0*m0);
  r0 = 2 * k0() * map_radius;
}

errc Stereographic::geo_xy (double *x, double *y, double lat, double lon) const
{
  double e = ellipsoid ().e ();
  double das = c2 * pow (tan (M_PI_4 + lat / 2.) * pow ((1 - e*sin (lat)) / (1 + e*sin (lat)), e/2), c1);
  double chi = 2 * atan (das) - M_PI_2;
  double dlam = lon*c1 - lam0s;
  double denom = 1 + sin (chi0) * sin (chi) + cos (chi0) * cos (chi) * cos (dlam);
  *x = false_east() + r0 * cos (chi) * sin (dlam) / denom;
  *y = false_north() + r0 * (sin (chi)*cos (chi0) - cos (chi)*sin (chi0)*cos (dlam)) / denom;
  return ERR_SUCCESS;
}

errc Stereographic::xy_geo (double x, double y, double *lat, double *lon) const
{
  //reduce coordinate relative to origin
  double xx = (x - false_east()) / k0();
  double yy = (y - false_north()) / k0();

  double ss = hypot (xx, yy);
  if (ss > 1e-20)
  {
    xx /= ss;
    yy /= ss;
  }
  else
  {
    xx = 1.;
    yy = 0.;
  }

  //calculate spherical coordinates
  double del = 2 * atan (ss / (2 * map_radius));
  double chi = asin (sin (chi0)*cos (del) + cos (chi0)*sin (del)*yy);
  double lams = lam0s + asin (xx*sin (del) / cos (chi));

  //ellipsoidal coordinates
  double phi = chi;
  double dif = 1;
  while (fabs (dif) > 1e-12)
  {
    double fact = pow ((1 - ellipsoid().e ()*sin (phi)) / (1 + ellipsoid().e ()*sin (phi)), ellipsoid().e () / 2);
    double tanf = tan (M_PI_4 + phi / 2);
    dif = c2 * pow (tanf*fact, c1) - tan (M_PI_4 + chi / 2);
    dif /= c1*c2*pow (tanf*fact, c1 - 1)*(fact / 2 / pow (cos (M_PI_4 + phi / 2), 2) -
      ellipsoid().e2 ()*sin (phi)*cos (phi)*tanf / (1 - ellipsoid().e2 ()*sin (phi)));
    phi -= dif;
  }
  *lat = phi;
  *lon = lams / c1;
  return ERR_SUCCESS;
}

double Stereographic::k (double lat, double lon) const
{
  double scale;

  double dlam = lon*c1 - lam0s;
  double das = c2 * pow (tan (M_PI_4 + lat / 2.) * pow ((1 - ellipsoid().e ()*sin (lat)) / (1 + ellipsoid().e ()*sin (lat)), ellipsoid().e () / 2), c1);
  double chi = 2 * atan (das) - M_PI_2;
  double denom = 1 + sin (chi0) * sin (chi) + cos (chi0) * cos (chi) * cos (dlam);
  double a_big = r0 / denom;

  scale = a_big*cos (chi) / (ellipsoid().a ()*ellipsoid().m (lat));
  return scale;
}

/*==============================================================================
                      Polar Stereographic projection 
==============================================================================*/

PolarStereo::PolarStereo (const Params& params) :
Projection (params)
{
  init ();
}

void PolarStereo::init ()
{
  double e = ellipsoid ().e ();
  double e2 = ellipsoid ().e2 ();

  if (fabs (fabs (ref_latitude ()) - M_PI / 2) > 1e-5)
  {
    //projection specified by latitude of standard parallel
    double tf;
    double sr = sin (ref_latitude ());
    double x = pow ((1 + e*sr) / (1 - e*sr), e/2);
    if (ref_latitude () < 0)
      tf = tan (M_PI / 4 + ref_latitude () / 2) / x;      //South polar case
    else
      tf = tan (M_PI / 4 - ref_latitude () / 2)*x;         //North polar case
    double mf = cos (ref_latitude ()) / sqrt (1 - e2*sr*sr);
    k_ = mf*sqrt (pow (1 + e, 1 + e) * pow (1 - e, 1 - e)) / (2 * tf);
  }
  else
    k_ = k0 ();
  rho1 = 2 * ellipsoid().a () * k_ / sqrt (pow (1 + e, 1 + e) * pow (1 - e, 1 - e));
  sc[0] = e2*(1 / 2. + e2*(5. / 24. + e2*(1. / 12. + 13.*e2 / 360.)));
  sc[1] = e2*e2*(7. / 48. + e2*(29. / 240. + e2*811. / 11520.));
  sc[2] = e2*e2*e2*(7. / 120. + e2*81. / 1120.);
  sc[3] = e2*e2*e2*e2*4279. / 161280.;
}

errc PolarStereo::geo_xy (double *x, double *y, double lat, double lon) const
{
  if (ref_latitude() < 0)
    *y = false_north() + rho (lat) * cos (lon - ref_longitude());
  else
    *y = false_north() - rho (lat) * cos (lon - ref_longitude());

  *x = false_east() + rho (lat) * sin (lon - ref_longitude());
  return ERR_SUCCESS;
}

errc PolarStereo::xy_geo (double x, double y, double *lat, double *lon) const
{
  double rhoprime = hypot (x - false_east(), y - false_north());
  double tprime = rhoprime / rho1;
  double chi;
  if (ref_latitude() < 0)
  {
    chi = 2 * atan (tprime) - M_PI / 2;
    if (x == false_east())
      *lon = ref_longitude();
    else
      *lon = lon_adjust (ref_longitude() + atan2 (x - false_east(), y - false_north()));
  }
  else
  {
    chi = M_PI / 2 - 2 * atan (tprime);
    if (x == false_east())
      *lon = ref_longitude();
    else
      *lon = lon_adjust (ref_longitude() + atan2 (x - false_east(), false_north() - y));
  }
  *lat = chi + sc[0] * sin (2 * chi) + sc[1] * sin (4 * chi) + sc[2] * sin (6 * chi) + sc[3] * sin (8 * chi);
  return ERR_SUCCESS;
}

double PolarStereo::k (double lat, double lon) const
{
  return rho (lat) / (ellipsoid().a ()*ellipsoid().m (lat));
}

double PolarStereo::rho (double lat) const
{
  double sphi = sin (lat);
  double e = ellipsoid ().e ();
  double t;
  if (ref_latitude() < 0)
    t = tan (M_PI / 4 + lat / 2) / pow (((1 + e*sphi) / (1 - e*sphi)), e/2); //south polar
  else
    t = tan (M_PI / 4 - lat / 2) * pow (((1 + e*sphi) / (1 - e*sphi)), e/2);

  return t*rho1;
}

#ifdef MLIBSPACE
};
#endif

/*!
  \file MER.CPP - Implementation of Mercator and SphericalMercator projections

*/

#include <geo/mer.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

static double sqrarg__;
#define SQR(a) ((sqrarg__=(a)) == 0.?0.:sqrarg__*sqrarg__)


Mercator::Mercator () :
  sfeq (0.)
{
}

Mercator::Mercator (const Params& params) :
  Projection (params)
{
  init ();
}

Mercator& Mercator::operator= (const Params& p)
{
  par = p;
  init ();
  return *this;
}

void Mercator::init ()
{
  if (ref_latitude () == M_PI / 2)
    throw errc (GEOERR_PARAM);
  sfeq = cos (ref_latitude ()) * ellipsoid ().rn (ref_latitude ()) / unit ();
}

errc Mercator::xy_geo (double x, double y, double *lat, double *lon) const
{
  double xtrue = x - false_east();
  double ytrue = y - false_north();
  double t = exp (-ytrue / sfeq);
  double phi = M_PI_2 - 2 * atan (t);
  double phidif = 1.;
  while (phidif > .5e-10)
  {
    double sphi;
    double p5 = M_PI_2 - 2. * atan (t * pow ((1. - ellipsoid().e () *(sphi = sin (phi))) / 
      (1. + ellipsoid().e () * sphi), ellipsoid().e () / 2.));
    phidif = fabs (phi - p5);
    phi = p5;
  }
  *lon = lon_adjust (ref_longitude() + xtrue / sfeq);
  *lat = phi;
  return ERR_SUCCESS;
}

errc Mercator::geo_xy (double *x, double *y, double lat, double lon) const
{
  double tau, slat;
  if (fabs (lat) == M_PI_2)
    return errc (GEOERR_SINGL);
  lon = lon_adjust (lon - ref_longitude());
  *x = lon * sfeq + false_east();
  slat = sin (lat);
  tau = sfeq / 2.*log ((1 + slat) / (1 - slat) * pow ((1 - ellipsoid().e () * slat) / (1 + ellipsoid().e () * slat), ellipsoid().e ()));
  *y = tau + false_north();
  return ERR_SUCCESS;
}

double Mercator::k (double lat, double lon) const
{
  return sqrt ((1 - ellipsoid().e2 ()*SQR (sin (lat))) / (1 - ellipsoid().e2 ()*SQR (sin (ref_latitude()))))*(cos (ref_latitude()) / cos (lat));
}


/*!
  \class CMapMercator
  This is a simplified %Mercator projection using equations for
  a spherical Earth. It is used by CMap charts (CM93 Version 2).

  The projections are always done using a sphere with radius 6378388.0 m.
*/
const double r0 = 6378388.0;      // Cmap Lat/Lon const

CMapMercator::CMapMercator ()
{
  par = Params (Ellipsoid (r0, 0.));
}

errc CMapMercator::xy_geo (double x, double y, double *lat, double *lon) const
{
  *lon = x / r0;
  *lat = 2.0 * atan (exp (y / r0)) - M_PI_2;
  return ERR_SUCCESS;
}

errc CMapMercator::geo_xy (double *x, double *y, double lat, double lon) const
{
  *x = lon * r0;
  *y = r0 * log (tan ((lat + M_PI_2) / 2.0));
  return ERR_SUCCESS;
}

double CMapMercator::k (double lat, double lon) const
{
  return 1/cos(lat);
}

#ifdef MLIBSPACE
}
#endif

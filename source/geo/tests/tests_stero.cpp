#include <utpp/utpp.h>
#include <geo/stereo.h>
#include <geo/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

/*
Source:
Coördinaattransformaties en kaartprojecties
Formules en parameters
3e herziene uitgave December 2000

http://kadaster.nl
*/
TEST (Netherlands)
{
  Projection::Params pp (Ellipsoid::BESSEL_1841);
  pp.k0 (0.9999079)
    .ref_longitude (DMS(5, 23, 15.5))
    .ref_latitude (DMS(52, 9, 22.178))
    .false_east (155000)
    .false_north (463000);

  const double 
    lat_ref = DMS(52, 12, 34.567),
    lon_ref = DMS(4, 23, 45.678),
    x_ref = 87232.211,
    y_ref = 469408.512;
  double lat, lon, x, y;

  Stereographic ost(pp);

  CHECK(!ost.geo_xy (&x, &y, lat_ref, lon_ref));
  CHECK_CLOSE (x_ref, x, 0.001);
  CHECK_CLOSE (y_ref, y, 0.001);


  CHECK (!ost.xy_geo (x_ref, y_ref, &lat, &lon));
  CHECK_CLOSE (lat_ref, lat, 0.1*MAS);
  CHECK_CLOSE (lon_ref, lon, 0.1*MAS);
}

/*
Source:
A manual for geodetic coordinate transformations in the maritime provinces, 
D.B. Thomson, E.J. Krakiwsky, R.R. Steeves, Technical Report No. 48, July 
1977, Department of Geodesy and Geomatics Engineering, University of New 
Brunswick, Canada.
http://gge.unb.ca/Pubs/TR48.pdf
*/
TEST (NewBrunswick)
{
  Projection::Params pp (Ellipsoid::CLARKE_1866);
  pp.k0 (0.999912)
    .ref_longitude (-DM (66, 30))
    .ref_latitude (DM (46, 30))
    .false_east (300000)
    .false_north (800000);

  const double 
    lat_ref = DMS(47, 03, 24.644),
    lon_ref = -DMS(65, 29, 03.453),
    x_ref = 377164.887,
    y_ref = 862395.774;

  double lat, lon, x, y;
  Stereographic ost(pp);

  //Forward transformation
  CHECK(!ost.geo_xy (&x, &y, lat_ref, lon_ref));
  CHECK_CLOSE (x_ref, x, 0.001);
  CHECK_CLOSE (y_ref, y, 0.001);

  //Inverse transformation
  CHECK(!ost.xy_geo (x_ref, y_ref, &lat, &lon));
  CHECK_CLOSE (lat_ref, lat, 1E-9);
  CHECK_CLOSE (lon_ref, lon, 1E-9);
}

/*
 Polar aspect with known k0. Source: Snyder p. 314, 317

 Synder gives x = -1573645.4. Manual calculation gives -1573645.25. The difference
 appears in the value of rho1. It is probably just a rounding error.
*/
TEST(PolarStereographic_k0)
{
  Projection::Params pp (Ellipsoid::INTERNATIONAL);
  pp.k0 (0.994)
    .ref_longitude (-DEG (100))
    .ref_latitude (-DEG (90));

  const double 
    lat_ref = -DEG(75),
    lon_ref = DEG(150),
    x_ref = -1573645.25,
    y_ref = -572760.1;

  double lat, lon, x, y;
  PolarStereo pst(pp);

  //Forward transformation
  CHECK(!pst.geo_xy (&x, &y, lat_ref, lon_ref));
  CHECK_CLOSE (x_ref, x, 0.1);
  CHECK_CLOSE (y_ref, y, 0.1);

  //Inverse transformation
  CHECK(!pst.xy_geo (x_ref, y_ref, &lat, &lon));
  CHECK_CLOSE (lat_ref, lat, 1E-7);
  CHECK_CLOSE (lon_ref, lon, 1E-7);
}

/* 
  Polar aspect with known phi_c not at the pole. Source Snyder p. 315
*/
TEST(PolarStereographic_phic)
{
  PolarStereo pst = Projection::Params ()
    .ellipsoid (Ellipsoid::INTERNATIONAL)
    .ref_longitude (-DEG (100))
    .ref_latitude (-DEG (71));

  const double 
    lat_ref = -DEG(75),
    lon_ref = DEG(150),
    x_ref = -1540033.6,
    y_ref = -560526.4,
    k_ref = 0.9896256;

  double lat, lon, x, y;

  //Forward transformation
  CHECK (!pst.geo_xy (&x, &y, lat_ref, lon_ref));
  CHECK_CLOSE (x_ref, x, 0.1);
  CHECK_CLOSE (y_ref, y, 0.1);

  //Inverse transformation
  CHECK (!pst.xy_geo (x_ref, y_ref, &lat, &lon));
  CHECK_CLOSE (lat_ref, lat, 1E-7);
  CHECK_CLOSE (lon_ref, lon, 1E-7);

  //Scale factor
  CHECK_CLOSE (k_ref, pst.k(lat_ref, lon_ref), 1e-7);
}


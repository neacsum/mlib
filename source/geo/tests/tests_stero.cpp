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
  PROJPARAMS gp = {
    6377397.155,                            // a
    299.1528128,                            // 1/f
    GEOPROJ_OST,                            // projection
    1.,                                     // unit
    0.9999079,                              // scale factor
    DMS(5, 23, 15.5),                       // central meridian
    DMS(52, 9, 22.178),                     // ref latitude
    0,                                      // north par.
    0,                                      // south par.
    0,                                      // azimuth
    155000,                                 // false easting
    463000};                                // false northing

  const double 
    lat_ref = DMS(52, 12, 34.567),
    lon_ref = DMS(4, 23, 45.678),
    x_ref = 87232.211,
    y_ref = 469408.512;
  double lat, lon, x, y;
  Stereographic ost(gp);

  CHECK_EQUAL (0, ost.GeoXY (&x, &y, lat_ref, lon_ref));
  CHECK_CLOSE (x_ref, x, 0.001);
  CHECK_CLOSE (y_ref, y, 0.001);


  CHECK_EQUAL (0, ost.XYGeo (x_ref, y_ref, &lat, &lon));
  CHECK_CLOSE (lat_ref, lat, 1E-9);
  CHECK_CLOSE (lon_ref, lon, 1E-9);
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
  PROJPARAMS gp = {
    6378206.4,                              // a
    294.978698200,                          // 1/f
    GEOPROJ_OST,                            // projection
    1.,                                     // unit
    0.999912,                               // scale factor
    -DM(66, 30),                            // central meridian
    DM(46, 30),                             // ref latitude
    0,                                      // north par.
    0,                                      // south par.
    0,                                      // azimuth
    300000,                                 // false easting
    800000};                                // false northing
  const double 
    lat_ref = DMS(47, 03, 24.644),
    lon_ref = -DMS(65, 29, 03.453),
    x_ref = 377164.887,
    y_ref = 862395.774;

  double lat, lon, x, y;
  Stereographic ost(gp);

  //Forward transformation
  CHECK_EQUAL (0, ost.GeoXY (&x, &y, lat_ref, lon_ref));
  CHECK_CLOSE (x_ref, x, 0.001);
  CHECK_CLOSE (y_ref, y, 0.001);

  //Inverse transformation
  CHECK_EQUAL (0, ost.XYGeo (x_ref, y_ref, &lat, &lon));
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
  PROJPARAMS pp = {
    6378388,                                // a
    297,                                    // 1/f
    GEOPROJ_PST,                            // projection
    1.,                                     // unit
    0.994,                                  // scale factor
    -DEG(100),                              // central meridian
    -DEG(90),                               // ref latitude
    0,                                      // north par.
    0,                                      // south par.
    0,                                      // azimuth
    0,                                      // false easting
    0};                                     // false northing
  const double 
    lat_ref = -DEG(75),
    lon_ref = DEG(150),
    x_ref = -1573645.25,
    y_ref = -572760.1;

  double lat, lon, x, y;
  PolarStereo pst(pp);

  //Forward transformation
  CHECK_EQUAL (0, pst.GeoXY (&x, &y, lat_ref, lon_ref));
  CHECK_CLOSE (x_ref, x, 0.1);
  CHECK_CLOSE (y_ref, y, 0.1);

  //Inverse transformation
  CHECK_EQUAL (0, pst.XYGeo (x_ref, y_ref, &lat, &lon));
  CHECK_CLOSE (lat_ref, lat, 1E-7);
  CHECK_CLOSE (lon_ref, lon, 1E-7);
}

/* 
  Polar aspect with known phi_c not at the pole. Source Snyder p. 315
*/
TEST(PolarStereographic_phic)
{
  PROJPARAMS pp = {
    6378388,                                // a
    297,                                    // 1/f
    GEOPROJ_PST,                            // projection
    1.,                                     // unit
    1.0,                                    // scale factor
    -DEG(100),                              // central meridian
    -DEG(71),                               // ref latitude
    0,                                      // north par.
    0,                                      // south par.
    0,                                      // azimuth
    0,                                      // false easting
    0};                                     // false northing
  const double 
    lat_ref = -DEG(75),
    lon_ref = DEG(150),
    x_ref = -1540033.6,
    y_ref = -560526.4,
    k_ref = 0.9896256;

  double lat, lon, x, y;
  PolarStereo pst(pp);

  //Forward transformation
  CHECK_EQUAL (0, pst.GeoXY (&x, &y, lat_ref, lon_ref));
  CHECK_CLOSE (x_ref, x, 0.1);
  CHECK_CLOSE (y_ref, y, 0.1);

  //Inverse transformation
  CHECK_EQUAL (0, pst.XYGeo (x_ref, y_ref, &lat, &lon));
  CHECK_CLOSE (lat_ref, lat, 1E-7);
  CHECK_CLOSE (lon_ref, lon, 1E-7);

  //Scale factor
  CHECK_CLOSE (k_ref, pst.k(lat_ref, lon_ref), 1e-7);
}


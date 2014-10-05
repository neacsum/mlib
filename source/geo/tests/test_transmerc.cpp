#include <utpp/utpp.h>

#include <geo/tranmerc.h>
#include <geo/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

//Snyder pag. 269-270
TEST (TransverseMercator_Forward)
{
  PROJPARAMS pp = {
    6378206.4,                              // a
    294.978698200,                          // 1/f
    GEOPROJ_TME,                            // projection
    1.,                                     // unit
    0.9996,                                 // scale factor
    -75*D2R,                                // central meridian
    0,                                      // ref latitude
    0,                                      // north par.
    0,                                      // south par.
    0,                                      // azimuth
    0,                                      // false easting
    0};                                     // false northing
  double lat = 40.5 * D2R;
  double lon = -73.5 * D2R;
  double x, y, k;
  int ret;

  TransverseMercator tm(pp);
  ret = tm.GeoXY (&x, &y, lat, lon);
  CHECK_EQUAL (0, ret);
  CHECK_CLOSE (127106.5, x, 0.1);
  CHECK_CLOSE (4484124.4, y, 0.1);

  k = tm.k(lat, lon);
  CHECK_CLOSE (0.9997989, k, 1e-7);

}

TEST (TransverseMercator_Inverse)
{
  PROJPARAMS pp = {
    6378206.4,                              // a
    294.978698200,                          // 1/f
    GEOPROJ_TME,                            // projection
    1.,                                     // unit
    0.9996,                                 // scale factor
    -75*D2R,                                // central meridian
    0,                                      // ref latitude
    0,                                      // north par.
    0,                                      // south par.
    0,                                      // azimuth
    0,                                      // false easting
    0};                                     // false northing
  double lat, lon;
  double x=127106.5, y=4484124.4;
  int ret;

  TransverseMercator tm(pp);
  ret = tm.XYGeo (x, y, &lat, &lon);
  CHECK_EQUAL (0, ret);
  CHECK_CLOSE (40.5, lat/D2R, 1e-6);
  CHECK_CLOSE (-73.5,lon/D2R, 1e-6);

}

//from old TEST.INI
TEST (TransverseMercator_own)
{
  PROJPARAMS pp = {
    6378137.000,                            // a
    298.257223563,                          // 1/f
    GEOPROJ_TME,                            // projection
    1.,                                     // unit
    0.9999,                                 // scale factor
    -73.5*D2R,                              // central meridian
    0,                                      // ref latitude
    0,                                      // north par.
    0,                                      // south par.
    0,                                      // azimuth
    304800,                                 // false easting
    0};                                     // false northing

  double lat=45*D2R, 
    lon = -72*D2R,
    x = 423058.45, 
    y = 4985540.61;

  double latr, lonr, xr, yr;

  TransverseMercator tm(pp);
  tm.GeoXY (&xr, &yr, lat, lon);
  CHECK_CLOSE (x, xr, 0.01);
  CHECK_CLOSE (y, yr, 0.01);

  tm.XYGeo (x, y, &latr, &lonr);
  CHECK_CLOSE (latr, lat, 1e-6);
  CHECK_CLOSE (lonr, lon, 1e-6);

}

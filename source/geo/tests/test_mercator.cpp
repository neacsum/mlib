#include <utpp/utpp.h>

#include <geo/mer.h>
#include <geo/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

/*
  OSGEO test vectors are from:
  http://svn.osgeo.org/geotools/branches/2.6.x/modules/library/referencing/src/test/resources/org/geotools/referencing/test-data/scripts/Mercator.txt
*/
SUITE (CMapTests)
{
  const double lat = 45 * D2R,
    lon = -72 * D2R,
    x = -8015318.753,
    y = 5621742.711;

  TEST (Forward)
  {
    CMapMercator prj;

    double xr, yr;
    prj.geo_xy (&xr, &yr, lat, lon);
    CHECK_CLOSE (x, xr, 1e-3);
    CHECK_CLOSE (y, yr, 1e-3);
  }

  TEST (Inverse)
  {
    double latr, lonr;

    CMapMercator prj;

    prj.xy_geo (x, y, &latr, &lonr);
    CHECK_CLOSE (lat, latr, 1e-7);
    CHECK_CLOSE (lon, lonr, 1e-7);
  }
}

SUITE (MercatorTests)
{
  const double lat = 35 * D2R,
    lon = -75 * D2R,
    x = 11688673.715,
    y = 4139145.663,
    k = 1.2194146;

  TEST (Forward)
  {
    double xr, yr, kr;

    Mercator prj = ProjParams (Ellipsoid::CLARKE_1866)
                    .ref_longitude (-180 * D2R);

    prj.geo_xy (&xr, &yr, lat, lon);
    CHECK_CLOSE (x, xr, 1e-3);
    CHECK_CLOSE (y, yr, 1e-3);
    kr = prj.k (lat, lon);
    CHECK_CLOSE (kr, k, 1e-7);
  }

  TEST (Inverse)
  {
    double latr, lonr;

    Mercator prj = ProjParams (Ellipsoid::CLARKE_1866)
                    .ref_longitude (-180 * D2R);

    prj.xy_geo (x, y, &latr, &lonr);
    CHECK_CLOSE (lat, latr, 1e-7);
    CHECK_CLOSE (lon, lonr, 1e-7);
  }

  TEST (OSGEO_Mercator_1SP)
  {
    double xr, yr;
    Mercator prj = ProjParams ()
                    .ref_longitude (-20 * D2R)
                    .false_east (500000);

    prj.geo_xy (&xr, &yr, 49.2166666666*D2R, -123.1*D2R);
    CHECK_CLOSE (-10977039.5007865, xr, 1e-4);
    CHECK_CLOSE (6279333.98057739, yr, 1e-4);
  }

  TEST (OSGEO_Mercator_2SP)
  {
    double xr, yr;

    Mercator prj = ProjParams()
                    .ref_longitude (45 * D2R)
                    .ref_latitude (49 * D2R)
                    .false_north (1000000);

    prj.geo_xy (&xr, &yr, 49.2166666666*D2R, -123.1*D2R);
    CHECK_CLOSE (-12300178.4624595, xr, 1e-4);
    CHECK_CLOSE (5127490.38951162, yr, 1e-4);
  }

  TEST (Singularity)
  {
    double xr, yr;
    Mercator prj  = ProjParams(Ellipsoid::CLARKE_1866);

    CHECK_THROW_EQUAL (prj.geo_xy (&xr, &yr, M_PI_2, 0), erc, GEOERR_SINGL);
  }
}

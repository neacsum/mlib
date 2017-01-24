#include <utpp/utpp.h>

#include <geo/lcc.h>
#include <mlib/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

SUITE (Lambert_Conformal_Conical)
{
  struct Snyder
  {
    Snyder ();
    Projection::Params par;
    double lat_check, lon_check;
    double x_check, y_check, k_check;
  };

  Snyder::Snyder () :
    lat_check (35 * D2R),
    lon_check (-75 * D2R),
    x_check (1894410.9),
    y_check (1564649.5),
    k_check (0.9970171),
    par (Ellipsoid::CLARKE_1866)
  {
    par.ref_longitude (-96 * D2R)
       .ref_latitude (23 * D2R)
       .north_latitude (33 * D2R)
       .south_latitude (45 * D2R);
  }

  TEST_FIXTURE (Snyder, Forward)
  {
    double x_result, y_result;
    int ret;
    Lambert lcc (par);
    ret = lcc.geo_xy (&x_result, &y_result, lat_check, lon_check);
    CHECK_EQUAL (0, ret);
    CHECK_CLOSE (x_check, x_result, 0.1);
    CHECK_CLOSE (y_check, y_result, 0.1);
  }

  TEST_FIXTURE (Snyder, Scale)
  {
    double k_result, h_result;
    Lambert lcc (par);
    k_result = lcc.k (lat_check, lon_check);
    CHECK_CLOSE (k_check, k_result, 1e-7);
    h_result = lcc.h (lat_check, lon_check);
    CHECK_CLOSE (k_check, h_result, 1e-7);
  }


  TEST_FIXTURE (Snyder, Inverse)
  {
    double lat_result, lon_result;
    int ret;
    Lambert lcc (par);
    ret = lcc.xy_geo (x_check, y_check, &lat_result, &lon_result);
    CHECK_EQUAL (0, ret);
    CHECK_CLOSE (lat_check, lat_result, 1e-7);
    CHECK_CLOSE (lon_check, lon_result, 1e-7);
  }

  /* Belgian Lambert-72. Parameters and test point from IGN website:
  http://www.ngi.be/FR/FR2-1-4.shtm */
  struct Lambert72
  {
    Lambert72 ();
    Lambert lcc;
    double lat_check, lon_check;
    double x_check, y_check, k_check;
  };

  Lambert72::Lambert72 () :
    lcc (Projection::Params (Ellipsoid::INTERNATIONAL)
    .ref_latitude (90*D2R)
    .ref_longitude (DMS(4, 22, 02.952))
    .north_latitude (DMS(51, 10, 0.00204))
    .south_latitude (DMS(49, 50, 0.00204))
    .false_east (150000.013)
    .false_north (5400088.438)),
    lat_check (DMS (50, 40, 46.461)),
    lon_check (DMS (5, 48, 26.533)),
    x_check (251763.204),
    y_check (153034.174)
  {
  }

  TEST_FIXTURE (Lambert72, Forward)
  {
    double x_result, y_result;
    int ret;
    ret = lcc.geo_xy (&x_result, &y_result, lat_check, lon_check);
    CHECK_EQUAL (0, ret);
    CHECK_CLOSE (x_check, x_result, 0.01);
    CHECK_CLOSE (y_check, y_result, 0.01);
  }

  TEST_FIXTURE (Lambert72, Inverse)
  {
    double lat_result, lon_result;
    int ret;
    ret = lcc.xy_geo (x_check, y_check, &lat_result, &lon_result);
    CHECK_EQUAL (0, ret);
    CHECK_CLOSE (lat_check, lat_result, 1e-7);
    CHECK_CLOSE (lon_check, lon_result, 1e-7);
  }
}


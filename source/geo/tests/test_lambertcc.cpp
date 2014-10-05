#include <utpp/utpp.h>

#include <geo/lambert.h>
#include <geo/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

/* Parameters for tests in Snyder p. 296, 297
  Clarke-1866 with 2 standard parallels*/
PROJPARAMS pp_snyder = {
  6378206.4,
  294.978698200,
  GEOPROJ_LCC,
  1.,
  1.0,
  -96*D2R,
  23*D2R,
  33*D2R,
  45*D2R,
  0,
  0.,
  0
};

struct SnyderFixture
{
  SnyderFixture ();
  Lambert lcc;
  double lat_check, lon_check;
  double x_check, y_check, k_check;
};

SnyderFixture::SnyderFixture () :
  lcc (pp_snyder),
  lat_check (35*D2R),
  lon_check (-75*D2R),
  x_check (1894410.9),
  y_check (1564649.5),
  k_check (0.9970171)
{
}

TEST_FIXTURE (SnyderFixture, LCCSynder_GeoXY)
{
  double x_result, y_result;
  int ret;
  ret = lcc.GeoXY (&x_result, &y_result, lat_check, lon_check);
  CHECK_EQUAL (0, ret);
  CHECK_CLOSE (x_check, x_result, 0.1);
  CHECK_CLOSE (y_check, y_result, 0.1);
}

TEST_FIXTURE (SnyderFixture, LCCSynder_Scale)
{
  double k_result, h_result;
  k_result = lcc.k (lat_check, lon_check);
  CHECK_CLOSE (k_check, k_result, 1e-7);
  h_result = lcc.h (lat_check, lon_check);
  CHECK_CLOSE (k_check, h_result, 1e-7);
}


TEST_FIXTURE (SnyderFixture, LCCSynder_XYGeo)
{
  double lat_result, lon_result;
  int ret;
  ret = lcc.XYGeo (x_check, y_check, &lat_result, &lon_result);
  CHECK_EQUAL (0, ret);
  CHECK_CLOSE (lat_check, lat_result, 1e-7);
  CHECK_CLOSE (lon_check, lon_result, 1e-7);
}

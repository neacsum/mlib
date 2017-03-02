#include <utpp/utpp.h>

#include <geo/albers.h>
#include <mlib/convert.h>

using namespace mlib;

//Fixture. Test vector from Snyder pg 292-293
struct Albers_Snyder
{
  Albers_Snyder ();
  double lat_check, lon_check, x_check, y_check, k_check, h_check;
  Albers prj;
};

Albers_Snyder::Albers_Snyder () :
  lat_check (35 * D2R),
  lon_check (-75 * D2R),
  x_check (1885472.7),
  y_check (1535925.0),
  k_check (0.9915546),
  h_check (1.0085173)
{
  prj = Projection::Params(Ellipsoid::CLARKE_1866)
    .ref_latitude (23 * D2R)
    .ref_longitude (-96 * D2R)
    .south_latitude (29.5 * D2R)
    .north_latitude (45.5 * D2R);
}

TEST_FIXTURE (Albers_Snyder, Albers_Forward)
{
  double x_result, y_result;
  int ret;
  ret = prj.geo_xy (&x_result, &y_result, lat_check, lon_check);
  CHECK_EQUAL (0, ret);
  CHECK_CLOSE (x_check, x_result, 0.1);
  CHECK_CLOSE (y_check, y_result, 0.1);
}

TEST_FIXTURE (Albers_Snyder, Albers_Scale)
{
  double k_result, h_result;
  k_result = prj.k (lat_check, lon_check);
  CHECK_CLOSE (k_check, k_result, 1e-7);
  h_result = prj.h (lat_check, lon_check);
  CHECK_CLOSE (h_check, h_result, 1e-7);
}


TEST_FIXTURE (Albers_Snyder, Albers_Inverse)
{
  double lat_result, lon_result;
  int ret;
  ret = prj.xy_geo (x_check, y_check, &lat_result, &lon_result);
  CHECK_EQUAL (0, ret);
  CHECK_CLOSE (lat_check, lat_result, 1e-7);
  CHECK_CLOSE (lon_check, lon_result, 1e-7);
}

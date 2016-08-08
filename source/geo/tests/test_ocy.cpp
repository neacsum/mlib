#include <utpp/utpp.h>

#include <geo/ocy.h>
#include <geo/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

/*
  Swiss CH03 system with an older test point
*/
struct Swiss_ocy
{
  Swiss_ocy ();

  Projection::Params par;
  double lam_check, phi_check, x_check, y_check;
};

Swiss_ocy::Swiss_ocy () :
  par (Ellipsoid::BESSEL_1841),
  phi_check (DMS(46, 52, 42.266489)),
  lam_check (DMS(7, 27, 58.427230)),
  x_check (602030.9709),
  y_check (191774.9438)
{
  par.ref_longitude (DMS (7, 26, 22.5))
    .ref_latitude (DMS(46, 57, 08.66))
    .false_east (600000)
    .false_north (200000);
}

TEST_FIXTURE (Swiss_ocy, Swiss_ocy_fwd)
{
  double xr, yr;
  ObliqueCylindrical ocy (par);
  CHECK (!ocy.geo_xy (&xr, &yr, phi_check, lam_check));

  CHECK_CLOSE (x_check, xr, 0.001);
  CHECK_CLOSE (y_check, yr, 0.001);
}

TEST_FIXTURE (Swiss_ocy, Swiss_ocy_inv)
{
  double phir, lamr;
  ObliqueCylindrical ocy (par);
  CHECK (!ocy.xy_geo (x_check, y_check, &phir, &lamr));

  CHECK_CLOSE (phi_check, phir, 0.1*MAS);
  CHECK_CLOSE (lam_check, lamr, 0.1*MAS);
}

/*
  Swiss CH03 with test vector from Federal Office of Topography
  (point name: station Rigi)
*/
struct Swiss2
{
  Swiss2 ();

  Projection::Params par;
  double lam_check, phi_check, x_check, y_check, k_check;
};
Swiss2::Swiss2 () :
  par (Ellipsoid::BESSEL_1841),
  phi_check (DMS (47, 03, 28.95659233)),
  lam_check (DMS (8, 29, 11.11127154)),
  x_check (679520.05),
  y_check (212273.44),
  k_check (1.000001852)
{
  par.ref_longitude (DMS (7, 26, 22.5))
    .ref_latitude (DMS (46, 57, 08.66))
    .false_east (600000)
    .false_north (200000);
}

TEST_FIXTURE (Swiss2, Swiss2_fwd)
{
  double xr, yr;
  ObliqueCylindrical ocy (par);
  CHECK (!ocy.geo_xy (&xr, &yr, phi_check, lam_check));

  CHECK_CLOSE (x_check, xr, 0.001);
  CHECK_CLOSE (y_check, yr, 0.001);
}

TEST_FIXTURE (Swiss2, Swiss2_scale)
{
  double kr;
  ObliqueCylindrical ocy (par);
  kr = ocy.k (phi_check, lam_check);
  CHECK_CLOSE (k_check, kr, 1e-7);
}

TEST_FIXTURE (Swiss2, Swiss2_inv)
{
  double phir, lamr;
  ObliqueCylindrical ocy (par);
  CHECK (!ocy.xy_geo (x_check, y_check, &phir, &lamr));

  CHECK_CLOSE (phi_check, phir, 0.1*MAS);
  CHECK_CLOSE (lam_check, lamr, 0.1*MAS);
}

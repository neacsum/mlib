#include <utpp/utpp.h>

#include <geo/azd.h>
#include <geo/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

/*
  Test vectors from Snyder page 338-339
*/
struct Snyder_polar {
  Snyder_polar ();

  ProjParams par;
  double lam_check, phi_check, x_check, y_check, k_check;
};

Snyder_polar::Snyder_polar () :
par (Ellipsoid::INTERNATIONAL),
phi_check (80 * D2R),
lam_check (5 *D2R),
x_check (1078828.2),
y_check (289071.2),
k_check (1.0050946)
{
  par.ref_latitude (90 * D2R)
    .ref_longitude (-100 * D2R);
}

TEST_FIXTURE (Snyder_polar, AZD_polar_fwd)
{
  double xr, yr;
  AzimuthEqDist azd (par);
  CHECK (!azd.geo_xy (&xr, &yr, phi_check, lam_check));
  CHECK_CLOSE (x_check, xr, 0.1);
  CHECK_CLOSE (y_check, yr, 0.1);
}

TEST_FIXTURE (Snyder_polar, AZD_polar_inv)
{
  double phir, lamr;
  AzimuthEqDist azd (par);
  CHECK (!azd.xy_geo (x_check, y_check, &phir, &lamr));
  CHECK_CLOSE (phi_check, phir, 1e-7);
  CHECK_CLOSE (lam_check, lamr, 1e-7);
}

TEST_FIXTURE (Snyder_polar, AZD_polar_scale)
{
  double kr;
  AzimuthEqDist azd (par);
  kr = azd.k (phi_check, lam_check);
  CHECK_CLOSE (k_check, kr, 1e-7);
}

struct Snyder_oblique {
  Snyder_oblique ();

  ProjParams par;
  double lam_check, phi_check, x_check, y_check;
};

Snyder_oblique::Snyder_oblique () :
par (Ellipsoid::CLARKE_1866),
phi_check (DMS(13, 20, 20.53846)),
lam_check (DMS(144, 38, 07.19265)),
x_check (37712.48),
y_check (35242.00)
{
  par.ref_latitude (DMS (13, 28, 20.87887))
    .ref_longitude (DMS (144, 44, 55.50))
    .false_east (50000.)
    .false_north (50000);
}

TEST_FIXTURE (Snyder_oblique, AZD_oblique_fwd)
{
  double xr, yr;
  AzimuthEqDist azd (par);
  CHECK (!azd.geo_xy (&xr, &yr, phi_check, lam_check));
  CHECK_CLOSE (x_check, xr, 0.1);
  CHECK_CLOSE (y_check, yr, 0.1);
}

TEST_FIXTURE (Snyder_oblique, AZD_oblique_inv)
{
  double phir, lamr;
  AzimuthEqDist azd (par);
  CHECK (!azd.xy_geo (x_check, y_check, &phir, &lamr));
  CHECK_CLOSE (phi_check, phir, 1e-7);
  CHECK_CLOSE (lam_check, lamr, 1e-7);
}

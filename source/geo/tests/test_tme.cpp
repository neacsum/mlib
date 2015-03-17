#include <utpp/utpp.h>

#include <geo/tme.h>
#include <geo/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

struct Snyder_tme
{
  Snyder_tme ();

  ProjParams par;
  double lam_check, phi_check, x_check, y_check, k_check;
};

Snyder_tme::Snyder_tme () :
par (Ellipsoid::CLARKE_1866),
phi_check (DM (40, 30)),
lam_check (-DM(73, 30)),
x_check (127106.5),
y_check (4484124.4),
k_check (0.9997989)
{
  par.k0 (0.9996)
    .ref_longitude (-75 * D2R);
}

TEST_FIXTURE (Snyder_tme, Snyder_tme_fwd)
{
  double xr, yr;
  TransverseMercator tm (par);
  CHECK (!tm.geo_xy (&xr, &yr, phi_check, lam_check));

  CHECK_CLOSE (x_check, xr, 0.1);
  CHECK_CLOSE (y_check, yr, 0.1);
}

TEST_FIXTURE (Snyder_tme, Snyder_tme_scale)
{
  double kr;
  TransverseMercator tm (par);

  kr = tm.k (phi_check, lam_check);
  CHECK_CLOSE (k_check, kr, 1e-7);
}

TEST_FIXTURE (Snyder_tme, Snyder_tme_inv)
{
  double phir, lamr;
  TransverseMercator tm (par);
  CHECK (!tm.xy_geo (x_check, y_check, &phir, &lamr));

  CHECK_CLOSE (phi_check, phir, 10.*MAS);
  CHECK_CLOSE (lam_check, lamr, 10.*MAS);
}

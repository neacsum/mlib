#include <utpp/utpp.h>

#include <geo/cas.h>
#include <geo/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

struct Snyder_cas
{
  Snyder_cas ();

  Projection::Params par;
  double lam_check, phi_check, x_check, y_check;
};

Snyder_cas::Snyder_cas () :
par (Ellipsoid::CLARKE_1866),
phi_check (DEG(43)),
lam_check (-DEG(73)),
x_check (163071.1),
y_check (335127.6)
{
  par.k0 (0.9996)
    .ref_latitude (DEG (40))
    .ref_longitude (-DEG(75));
}

TEST_FIXTURE (Snyder_cas, Snyder_cas_fwd)
{
  double xr, yr;
  Cassini cas (par);
  CHECK (!cas.geo_xy (&xr, &yr, phi_check, lam_check));

  CHECK_CLOSE (x_check, xr, 0.1);
  CHECK_CLOSE (y_check, yr, 0.1);
}

TEST_FIXTURE (Snyder_cas, Snyder_cas_inv)
{
  double phir, lamr;
  Cassini cas (par);
  CHECK (!cas.xy_geo (x_check, y_check, &phir, &lamr));

  CHECK_CLOSE (phi_check, phir, 20.*MAS);
  CHECK_CLOSE (lam_check, lamr, 20.*MAS);
}

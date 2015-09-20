#include <utpp/utpp.h>

#include <geo/polycon.h>
#include <geo/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

/* Snyder page 274. This is the variant A numerical example (with 2 points on
central line converted into a variant B by back-calculating the longitude
of central point. The skew azimuth (alpha c) is given directly in the manual*/
struct Snyder_polyconic
{
  Snyder_polyconic ();

  Projection::Params par;
  double lam_check, phi_check, x_check, y_check, h_check;
};

Snyder_polyconic::Snyder_polyconic () :
par (Ellipsoid::CLARKE_1866),
phi_check (DEG(40)),
lam_check (-DEG(75)),
x_check (1776774.5),
y_check (1319657.8),
h_check (1.0393954)
{
  par.ref_latitude (DEG(30))
    .ref_longitude (-DEG(96));
}

TEST_FIXTURE (Snyder_polyconic, Snyder_polyconic_fwd)
{
  double hr;
  double xr, yr;
  Polyconic pol (par);
  CHECK (!pol.geo_xy (&xr, &yr, phi_check, lam_check));

  CHECK_CLOSE (x_check, xr, 0.1);
  CHECK_CLOSE (y_check, yr, 0.1);

  hr = pol.h (phi_check, lam_check);
  CHECK_CLOSE (h_check, hr, 1e-7);
}

TEST_FIXTURE (Snyder_polyconic, Snyder_polyconic_inv)
{
  double phir, lamr;
  Polyconic pol (par);
  CHECK (!pol.xy_geo (x_check, y_check, &phir, &lamr));

  CHECK_CLOSE (phi_check, phir, 10*MAS);
  CHECK_CLOSE (lam_check, lamr, 10*MAS);
}

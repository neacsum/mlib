#include <utpp/utpp.h>

#include <geo/ome.h>
#include <mlib/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

/* Snyder page 274. This is the variant A numerical example (with 2 points on
central line converted into a variant B by back-calculating the longitude
of central point. The skew azimuth (alpha c) is given directly in the manual*/
struct Snyder_hom
{
  Snyder_hom ();

  Projection::Params par;
  double lam_check, phi_check, x_check, y_check, k_check;
};

Snyder_hom::Snyder_hom () :
  par (Ellipsoid::CLARKE_1866),
  phi_check (DM(40,48)),
  lam_check (-74*D2R),
  x_check (963436.1),
  y_check (4369142.8),
  k_check (1.0307554)
{
  par.k0 (0.9996)
    .ref_latitude (40 * D2R)
    .ref_longitude (-100.9971878*D2R) //back calculated
    .skew_azimuth (-56.9466070*D2R)
    .false_east (4000000)
    .false_north (500000);
}

TEST_FIXTURE (Snyder_hom, Snyder_hom_fwd)
{
  double kr;
  double xr, yr;
  Hotine hom (par);
  CHECK (!hom.geo_xy (&xr, &yr, phi_check, lam_check));

  CHECK_CLOSE (x_check, xr, 0.1);
  CHECK_CLOSE (y_check, yr, 0.1);

  kr = hom.k (phi_check, lam_check);
  CHECK_CLOSE (k_check, kr, 1e-7);
}


/*
Test vector from NGS datasheet (PID UW8043)
*/
struct Juneau {
  Juneau ();

  Projection::Params par;
  double lam_check, phi_check, x_check, y_check, k_check;
};

Juneau::Juneau () :
  par (Ellipsoid::WGS_84),
  phi_check (DMS(58, 17, 57.74857)),
  lam_check (-DMS(134, 24, 39.09819)),
  x_check (775034.944),
  y_check (720035.558),
  k_check (.99993319)
{
  par.ref_latitude (57 * D2R)
    .ref_longitude (-DM(133, 40))
    .k0 (0.9999)
    .skew_azimuth (-DMS(36, 52, 11.6315))
    .false_east (5000000)
    .false_north (-5000000);
}

TEST_FIXTURE (Juneau, Juneau_fwd)
{
  double xr, yr;
  Hotine hom (par);
  CHECK (!hom.geo_xy (&xr, &yr, phi_check, lam_check));
  CHECK_CLOSE (x_check, xr, 0.001);
  CHECK_CLOSE (y_check, yr, 0.001);
}

TEST_FIXTURE (Juneau, Juneau_inv)
{
  double phir, lamr;
  Hotine hom (par);
  CHECK (!hom.xy_geo (x_check, y_check, &phir, &lamr));
  CHECK_CLOSE (phi_check, phir, 0.1*MAS);
  CHECK_CLOSE (lam_check, lamr, 0.1*MAS);
}

TEST_FIXTURE (Juneau, Juneau_scale)
{
  double kr;
  Hotine hom (par);
  kr = hom.k (phi_check, lam_check);
  CHECK_CLOSE (k_check, kr, 1e-7);
}

/* Test point from EPSG Guidance Note 7
*/

struct EPSG_rso {
  EPSG_rso ();

  Projection::Params par;
  double lam_check, phi_check, x_check, y_check;
};

EPSG_rso::EPSG_rso () :
  par (Ellipsoid::EVEREST_SABAH_SARWAK),
  phi_check (DMS (5, 23, 14.1129)),
  lam_check (DMS (115, 48, 19.8196)),
  x_check (679245.73),
  y_check (596562.78)
{
  par.ref_latitude (4 * D2R)
    .ref_longitude (115 * D2R)
    .skew_azimuth (DMS (53, 18, 56.9537))
    .k0 (0.99984);
}

TEST_FIXTURE (EPSG_rso, EPSG_rso_fwd)
{
  double xr, yr;
  RSO rso (par);
  CHECK (!rso.geo_xy (&xr, &yr, phi_check, lam_check));
  CHECK_CLOSE (x_check, xr, 0.01);
  CHECK_CLOSE (y_check, yr, 0.01);
}

TEST_FIXTURE (EPSG_rso, EPSG_rso_inv)
{
  double phir, lamr;
  RSO rso (par);
  CHECK (!rso.xy_geo (x_check, y_check, &phir, &lamr));
  CHECK_CLOSE (phi_check, phir, 1*MAS);
  CHECK_CLOSE (lam_check, lamr, 1*MAS);
}

#include <utpp/utpp.h>
#include <mlib/mlib.h>
#pragma hdrstop

using namespace mlib;

SUITE (Convert)
{
  TEST (dm_dms_deg)
  {
    CHECK (DM (12, 30) == DEG (12.5));
    CHECK (DMS (12, 34, 60) == DM (12, 35));
    CHECK (DM (12, 60) == DEG (13));
    CHECK_CLOSE (DMS (0, 0, 1), 1000._mas, 1e-15);
  }

  TEST (oplit)
  {
    CHECK_EQUAL (DEG (12.5), 12.5_deg);
    CHECK_EQUAL (1852, 1_nmi);
    CHECK_EQUAL (1200., 3937_ftUS);
    CHECK_CLOSE (1_deg, 60_arcmin, 1e-10);
    CHECK_CLOSE (1_deg, 3600_arcsec, 1e-10);
    CHECK_CLOSE (1_arcsec, 1000_mas, 1e-10);
  }

  TEST (atodeg)
  {
    CHECK_EQUAL (DMS2deg (12'34'56), atodeg ("12D34M56"));
    CHECK_EQUAL (-DMS2deg (12'34'56), atodeg ("12D34M56S"));

    CHECK_EQUAL (DMS (12, 34, 56.78)/D2R, atodeg ("12D34M56.78"));
    CHECK_EQUAL (-DMS (12, 34, 56.78)/D2R, atodeg ("12D34M56.78S"));

    //alternate separators
    CHECK_EQUAL (DMS (12, 34, 56)/D2R, atodeg (u8"12°34'56"));

    //empty string
    CHECK_EQUAL (0, atodeg (""));

    //degrees minutes
    CHECK_CLOSE (DM2deg (12'34.56), atodeg ("12D34.56M"), 1e-7);
    CHECK_CLOSE (-DM2deg (12'34.56), atodeg ("-12D34.56M"), 1e-7);

    //decimal degrees
    CHECK_CLOSE (12.3456, atodeg ("12.3456"), 1e-4);
    CHECK_CLOSE (-12.3456, atodeg ("-12.3456"), 1e-4);
    CHECK_CLOSE (-12.3456, atodeg ("12.3456W"), 1e-4);
  }

  TEST (degtoa)
  {
    CHECK_EQUAL (u8"12°34'56.00\"N", degtoa (DMS (12, 34, 56) / D2R, deg_fmt::seconds, true, 2));
    CHECK_EQUAL (u8"12°34'56.00\"S", degtoa (-DMS (12, 34, 56) / D2R, deg_fmt::seconds, true, 2));
    CHECK_EQUAL (u8"012°34.50'E", degtoa (DMS (12, 34, 30) / D2R, deg_fmt::minutes, false, 2));
    CHECK_EQUAL (u8"012°34.50'W", degtoa (-DMS (12, 34, 30) / D2R, deg_fmt::minutes, false, 2));
    CHECK_EQUAL (u8"012.3457°E", degtoa (12.345678, deg_fmt::degrees, false, 4));
    CHECK_EQUAL (u8"12.3457°N", degtoa (12.345678, deg_fmt::degrees, true, 4));
  }
}
﻿#include <utpp/utpp.h>
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
    CHECK_EQUAL (1852, 1._nmi);
    CHECK_EQUAL (1200., 3937._ftus);
  }

  TEST (atodeg)
  {
    CHECK_EQUAL (DMS (12, 34, 56)/D2R, atodeg ("12D34M56"));
    CHECK_EQUAL (-DMS (12, 34, 56)/D2R, atodeg ("12D34M56S"));

    CHECK_EQUAL (DMS (12, 34, 56.78)/D2R, atodeg ("12D34M56.78"));
    CHECK_EQUAL (-DMS (12, 34, 56.78)/D2R, atodeg ("12D34M56.78S"));

    //alternate separators
    CHECK_EQUAL (DMS (12, 34, 56)/D2R, atodeg (u8"12°34'56"));

    //empty string
    CHECK_EQUAL (0, atodeg (""));

    //degrees minutes
    CHECK_EQUAL (DM (12, 34.56)/D2R, atodeg ("12D34.56M"));
    CHECK_EQUAL (-DM (12, 34.56) / D2R, atodeg ("-12D34.56M"));

    //decimal degrees
    CHECK_CLOSE (12.3456, atodeg ("12.3456"), 1e-4);
    CHECK_CLOSE (-12.3456, atodeg ("-12.3456"), 1e-4);
    CHECK_CLOSE (-12.3456, atodeg ("12.3456W"), 1e-4);
  }

  TEST (degtoa)
  {
    CHECK_EQUAL (u8"12°34'56.00\"N", degtoa (DMS (12, 34, 56) / D2R, LL_SEC | LL_LAT, 2));
    CHECK_EQUAL (u8"12°34'56.00\"S", degtoa (-DMS (12, 34, 56) / D2R, LL_SEC | LL_LAT, 2));
    CHECK_EQUAL (u8"012°34.50'E", degtoa (DMS (12, 34, 30) / D2R, LL_MIN, 2));
    CHECK_EQUAL (u8"012°34.50'W", degtoa (-DMS (12, 34, 30) / D2R, LL_MIN, 2));
    CHECK_EQUAL (u8"012.3457°E", degtoa (12.345678, 0, 4));
    CHECK_EQUAL (u8"12.3457°N", degtoa (12.345678, LL_LAT, 4));
  }
}
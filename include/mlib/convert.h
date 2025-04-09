/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

/*!
  \file convert.h Conversion functions and frequently used constants
  \addtogroup convert Angle and Unit Conversion
  @{
*/
#pragma once

#include <math.h>
#include <numbers>
#include <string>
#include <cmath>

#ifndef M_PI
/// Older name for `std::numbers::pi`
#define M_PI std::numbers::pi
#endif

// Useful constants

/// Degrees to radians conversion factor
constexpr double D2R = M_PI / 180.;

/// Semimajor axis of WGS84 ellipsoid
constexpr double A_WGS84 = 6378137.000;

/// Flattening of WGS84 ellipsoid
constexpr double F_WGS84 = 0.003352810664747;

/// Inverse of flattening for WGS84 ellipsoid
constexpr double F1_WGS84 = 298.257223563;

#define NM2M     1852.                     ///< Nautical mile to meters conversion factor
#define MPS2KNOT (3600. / NM2M)            ///< Meters/sec to knots conversion factor
#define USFOOT2M (1200. / 3937.)           ///< US Survey foot to meters conversion factor

/// milli-arcsecond
constexpr double MAS = M_PI / (180 * 3600000.);


/// US survey foot literal operator  converts a value in US survey feet to meters
/// @{
constexpr double operator"" _ftUS (long double ftus)
{
  return ftus * USFOOT2M;
}

constexpr double operator"" _ftUS (unsigned long long ftus)
{
  return ftus * USFOOT2M;
}
/// @}

/// Nautical miles literal operator  converts a value in nautical miles to meters
/// @{
constexpr double operator"" _nmi (long double nmi)
{
  return nmi * NM2M;
}

constexpr double operator"" _nmi (unsigned long long nmi)
{
  return nmi * NM2M;
}
/// @}

// ================== Degrees conversion functions ==========================

// Decimal degrees---------------------------------------------------------

/// Convert decimal degrees to radians
constexpr double DEG (double dd)
{
  return dd * D2R;
}

/// Convert decimal degrees to radians
constexpr double D2rad (double dd)
{
  return dd * D2R;
}

/// Convert radians to decimal degrees
constexpr double rad2D (double r)
{
  return r / D2R;
}

// Degrees, minutes --------------------------------------------------------

/// Convert degrees, minutes to radians
constexpr double DM (double dd, double mm)
{
  return (dd + mm / 60.) * D2R;
}

/// Convert degrees, minutes (DDMM.mmm) to decimal degrees
constexpr double DM2deg (double ddmm)
{
  int sign = (ddmm >= 0) ? 1 : -1;
  if (ddmm < 0)
    ddmm = -ddmm;
  int deg = (int)(ddmm / 100.);
  ddmm -= (double)deg * 100.;
  return sign * (deg + ddmm / 60.);
}

/// Convert decimal degrees to degrees, minutes (DDMM.mmm)
constexpr double deg2DM (double dd)
{
  int deg = (int)dd;
  return (dd - deg) * 60. + deg * 100.;
}

/// Convert from radians to degrees, minutes (DDMM.mmm)
constexpr double rad2DM (double rad)
{
  return deg2DM (rad / D2R);
}

/// Convert degrees, minutes (DDMM.mmm) to radians
constexpr double DM2rad (double val)
{
  return DM2deg (val) * D2R;
}

// Degrees, minutes, seconds ------------------------------------------------

/// Convert degrees, minutes seconds to radians
constexpr double DMS (double dd, double mm, double ss)
{
  return (dd + mm / 60. + ss / 3600.) * D2R;
}

/// Convert degrees, minutes, seconds (DDMMSS.sss) to decimal degrees
constexpr double DMS2deg (double dms)
{
  int sign = (dms >= 0) ? 1 : -1;
  if (dms < 0)
    dms = -dms;
  int deg = (int)(dms / 10000.);
  dms -= (double)deg * 10000;
  int min = (int)(dms / 100.);
  dms -= (double)min * 100.;
  return sign * (deg + min / 60. + dms / 3600.);
}

/// Convert degrees, minutes, seconds (DDMMSS.sss) to radians
constexpr double DMS2rad (double dms)
{
  return DMS2deg (dms) * D2R;
}

// =================== Angle units literal operators =========================

/// Degrees literal operator converts a value in degrees to radians
/// @{
constexpr double operator"" _deg (long double deg)
{
  return deg * D2R;
}

constexpr double operator"" _deg (unsigned long long deg)
{
  return deg * D2R;
}
/// @}

/// Minutes literal operator converts a value in arc-minutes to radians
/// @{
constexpr double operator"" _arcmin (long double min)
{
  return min/60. * D2R;
}

constexpr double operator"" _arcmin (unsigned long long min)
{
  return min/60. * D2R;
}
/// @}

/// Degrees-minutes literal operator converts a value to radians
///@{
constexpr double operator ""_dm (long double val)
{
  return DM2rad (val);
}

constexpr double operator ""_dm (unsigned long long val)
{
  return DM2rad ((double)val);
}
///@}

/// Degrees-minutes-seconds literal operator converts a value to radians
///@{
constexpr double operator ""_dms (long double val)
{
  return DMS2rad (val);
}

constexpr double operator ""_dms (unsigned long long val)
{
  return DMS2rad ((double)val);
}
///@}

/// Seconds literal operator converts a value in arc-seconds to radians
/// @{
constexpr double operator"" _arcsec (long double sec)
{
  return sec/3600. * D2R;
}

constexpr double operator"" _arcsec (unsigned long long sec)
{
  return sec/3600. * D2R;
}
/// @}

/// Milli-arcseconds literal operator converts a value in thousandth of
/// arc-seconds to radians
/// @{
constexpr double operator"" _mas (long double mas)
{
  return mas * MAS;
}

constexpr double operator"" _mas (unsigned long long mas)
{
  return mas * MAS;
}
/// @}


namespace mlib {
/// Reduces a degrees value to [0,360) interval
double deg_reduce (double value);

/// Formatting options for degtoa() function
enum deg_fmt
{
  degrees = 0, ///< Decimal degrees (DD.dddd°)
  minutes = 1, ///< Degrees, minutes format (DD°MM.mmmm')
  seconds = 2 ///< Degrees, minutes, seconds format (DD°MM'SS.sss")
};

/// Conversion from degrees to a string.
std::string degtoa (double degrees, deg_fmt format, bool latitude, int precision);

/// Conversion from string to decimal degrees
double atodeg (const std::string& str);

/*!
   A handy template to get sin and cos in a single function call

   Using structured bindings (C++17), it can be called like:
\code{.cpp}
  auto [s, c] = sincos (M_PI/4.);
\endcode
*/
template <typename T>
std::pair<T, T> sincos (T val)
{
  return {std::sin (val), std::cos (val)};
}
} // namespace mlib

/// @}
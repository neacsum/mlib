#pragma once
/*!
  \file convert.h Conversion functions and frequently used constants


  \addtogroup other Other Functions
@{
*/

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#include <math.h>
#endif
#include <string>

// Useful constants
#define D2R      0.01745329251994          ///< Degrees to radians conversion factor
#define A_WGS84  6378137.000               ///< Semimajor axis of WGS84 ellipsoid
#define F_WGS84  0.003352810664747         ///< Flattening of WGS84 ellipsoid
#define F1_WGS84 298.257223563             ///< Inverse of flattening for WGS84 ellipsoid
#define NM2M     1852.                     ///< Nautical mile to meters conversion factor
#define MPS2KNOT (3600. / NM2M)            ///< Meters/sec to knots conversion factor
#define USFOOT2M (1200. / 3937.)           ///< US Survey foot to meters conversion factor
#define MAS      (M_PI / (180 * 3600000.)) ///< milli-arcsecond

/// Degrees literal operator converts a value to radians
constexpr double operator"" _deg (long double deg)
{
  return deg * D2R;
}

constexpr double operator"" _deg (unsigned long long deg)
{
  return deg * D2R;
}

/// Milli-arcseconds literal operator converts a value to radians
constexpr double operator"" _mas (long double mas)
{
  return mas * MAS;
}

/// US survey foot literal operator  converts a value to meters
constexpr double operator"" _ftus (long double ftus)
{
  return ftus * USFOOT2M;
}

/// Nautical miles literal operator  converts a value to meters
constexpr double operator"" _nmi (long double nmi)
{
  return nmi * NM2M;
}

/// Convert decimal degrees to radians
constexpr double DEG (double dd)
{
  return dd * D2R;
}

/// Convert degrees, minutes to radians
constexpr double DM (double dd, double mm)
{
  return (dd + mm / 60.) * D2R;
}

/// Convert degrees, minutes seconds to radians
constexpr double DMS (double dd, double mm, double ss)
{
  return (dd + mm / 60. + ss / 3600.) * D2R;
}

/// Conversion to decimal degrees from DDMM.mmm
constexpr double DMD2deg (double value)
{
  int sign = (value >= 0) ? 1 : -1;
  if (value < 0)
    value = -value;
  int deg = (int)(value / 10000.);
  value -= -(double)deg * 10000;
  int min = (int)(value / 100.);
  value -= (double)min * 100.;
  return sign * (deg + min / 60. + value / 3600.);
}

/// Conversion to decimal degrees from DDMMSS.ssss
constexpr double DMS2deg (double value)
{
  int sign = (value >= 0) ? 1 : -1;
  if (value < 0)
    value = -value;
  int deg = (int)(value / 100.);
  value -= (double)deg * 100.;
  return sign * (deg + value / 60.);
}

/// Conversion from decimal degrees to degrees, minutes (DDMM.mmm)
constexpr double deg2DMD (double value)
{
  int deg = (int)value;
  return (value - deg) * 60. + deg * 100.;
}

/// Conversion from degrees to radians
constexpr double D2rad (double val)
{
  return val * D2R;
}

/// Conversion from degrees, minutes (DDMM.mmm) to radians
constexpr double DMD2rad (double val)
{
  return DMD2deg (val) * D2R;
}

/// Conversion from degrees, minutes, seconds (DDMMSS.sss) to radians
constexpr double DMS2rad (double val)
{
  return DMS2deg (val) * D2R;
}

/// Conversion from radians to degrees, minutes (DDMM.mmm)
constexpr double rad2DMD (double val)
{
  return deg2DMD (val / D2R);
}

namespace mlib {
/// Reduces a degrees value to [0,360) interval
double deg_reduce (double value);

// formatting flags for degtoa function
#define LL_MIN 0x01 ///< Degrees, minutes format (DD°MM.mmmm')
#define LL_SEC 0x02 ///< Degrees, minutes, seconds format (DD°MM'SS.sss")
#define LL_LAT 0x04 ///< Latitude angle

std::string degtoa (double degrees, int flags, int precision);

double atodeg (const std::string& str);
///@}

} // namespace mlib

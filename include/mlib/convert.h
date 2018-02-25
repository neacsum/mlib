#pragma once
/*!
  \file CONVERT.H Conversion functions and frequently used constants


  \addtogroup other Other Functions
@{
*/

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#include <math.h>
#endif

// Useful constants
#define D2R       0.01745329251994      ///< Degrees to radians conversion factor
#define MPS2KNOT  1.94384449244         ///< Meters/sec to knots conversion factor
#define A_WGS84   6378137.000           ///< Semimajor axis of WGS84 ellipsoid
#define F_WGS84   0.003352810664747     ///< Flattening of WGS84 ellipsoid
#define F1_WGS84  298.257223563         ///< Inverse of flattening for WGS84 ellipsoid
#define NM2M      1852                  ///< Nautical mile to meters conversion factor
#define USFOOT    0.3048006096          ///< US Survey foot to meters conversion factor
#define MAS       (M_PI/(180*3600000.)) ///< milli-arcsecond

/// New syntax for degrees to radians conversion
constexpr double operator "" _deg (long double deg)
{
  return deg * D2R;
}

/// New syntax for milli-arcseconds to radians conversion
constexpr double operator "" _mas (long double mas)
{
  return mas * MAS;
}

/// New syntax for US Survey foot to meters conversion
constexpr double operator "" _ftus (long double ftus)
{
  return ftus * USFOOT;
}

/// New syntax for Nautical Miles to meters conversion
constexpr double operator "" _nmi (long double nmi)
{
  return nmi * NM2M;
}

/// Convert decimal degrees to radians
#define DEG(dd) ((dd)*D2R)

///Convert degrees, minutes to radians
#define DM(dd,mm) (((dd)+(mm)/60.)*D2R)

///Convert degrees, minutes seconds to radians
#define DMS(dd,mm,ss) (((dd)+(mm)/60.+(ss)/3600.)*D2R)

/// Conversion to decimal degrees from DDMM.mmm
double DMD2deg (double);

///Conversion to decimal degrees from DDMMSS.ssss
double DMS2deg (double);

///Conversion from decimal degrees to degrees, minutes (DDMM.mmm)
double deg2DMD (double value);

/// Conversion from degrees to radians
inline double D2rad (double val) { return val * D2R; }

/// Conversion from degrees, minutes (DDMM.mmm) to radians
inline double DMD2rad (double val) { return DMD2deg(val) * D2R; }

/// Conversion from degrees, minutes, seconds (DDMMSS.sss) to radians
inline double DMS2rad (double val) { return DMS2deg(val) * D2R; }

/// Conversion from radians to degrees, minutes (DDMM.mmm)
inline double rad2DMD (double val) { return deg2DMD (val/D2R); }
///@}


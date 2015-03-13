#pragma once
/*!
  \file CONVERT.H Conversion functions and frequently used constants


  \addtogroup  convert Conversion Functions and Useful Constants
@{
*/

// Useful constants
#define D2R       0.01745329251994      ///< Degrees to radians conversion factor
#define MPS2KNOT  1.94384449244         ///< Meters/sec to knots conversion factor
#define A_WGS84   6378137.000           ///< Semimajor axis of WGS84 ellipsoid
#define F_WGS84   0.003352810664747     ///< Flattening of WGS84 ellipsoid
#define F1_WGS84  298.257223563         ///< Inverse of flattening for WGS84 ellipsoid
#define NM2M      1852                  ///< Nautical mile to meters conversion factor
#define USFOOT    0.3048006096          ///< US Survey foot to meters conversion factor


#define DEG(dd) ((dd)*D2R)
#define DM(dd,mm) (((dd)+(mm)/60.)*D2R)
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


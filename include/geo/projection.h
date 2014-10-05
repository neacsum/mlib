#pragma once
/*!
  \file PROJECTI.H	- Projection class definition

*/

#include "ellip.h"
#include "geostruct.h"

#include <mlib/errorcode.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

class Projection : public Ellipsoid
{
public:
  Projection(  PROJPARAMS& pp );

  /// Return projection's name
  virtual const char *Name() const = 0;

  /// Return projection's identifier
  virtual geoproj Id() const = 0;

  /// Convert from XY to geographical coordinates
  virtual errc XYGeo(double x, double y, double *lat, double *lon) const = 0;

  /// Convert from geographical to XY coordinates
  virtual errc GeoXY(double *x, double *y, double lat, double lon) const = 0;

  /// Return average scale factor at a given point
  virtual errc Scale(double x, double y, double *scale);

  /// Return scale factor along the latitude
  virtual double k (double lat, double lon) const;

  /// Return scale factor along meridian
  virtual double h (double lat, double lon) const;

  double Unit() const;
  double ScaleFactor() const;
  double CentralMeridian() const;
  double ReferenceLatitude() const;
  double NorthParallel() const;
  double SouthParallel() const;
  double FalseEast() const;
  double FalseNorth() const;
  double SkewAzimuth() const;
  const char *EllipsoidName() const;

protected:
  double LonAdjust( double lon ) const;
  double k_;
  double central_meridian_;
  double ref_latitude_;
  double north_parallel_, south_parallel_;
  double skew_azimuth_;
  double unit_;
  double false_east_, false_north_;
};

/*==================== INLINE FUNCTIONS ===========================*/

/// Return scale factor at origin
inline 
double Projection::ScaleFactor() const { return k_; };

/// Return central meridian
inline 
double Projection::CentralMeridian() const { return central_meridian_; };

/// Return conversion factor from XY units to meters
inline 
double Projection::Unit() const { return unit_; };

/// Return reference latitude
inline 
double Projection::ReferenceLatitude() const { return ref_latitude_; };

/// Return north parallel
inline 
double Projection::NorthParallel() const { return north_parallel_; };

/// Return south parallel
inline
double Projection::SouthParallel() const { return south_parallel_; };

/// Return X (easting) value at origin
inline 
double Projection::FalseEast() const { return false_east_; };

/// Return Y (northing) value at origin
inline 
double Projection::FalseNorth() const { return false_north_; };

/// Return azimuth of skew
inline
double Projection::SkewAzimuth() const { return skew_azimuth_; };

/// Return ellipsoid's name or NULL if not known
inline 
const char *Projection::EllipsoidName() const { return Ellipsoid::Name(); };

/*!
  Default implementation for scale along the meridian returns the same value as
  the scale along the latitude k. This is true only for conformal projections
  and any derived projection that is not conformal would have to re-implement
  this function.
*/
inline
double Projection::h (double lat, double lon) const {return k(lat,lon);};

inline
double Projection::k (double lat, double lon) const {return 1.;};

///  Return an adjusted longitude between -M_PI and M_PI.
inline
double Projection::LonAdjust( double lon ) const
{
  return ((lon>=0)?1:-1)*(fmod(fabs(lon)+M_PI, 2*M_PI)-M_PI);
}

#ifdef MLIBSPACE
};
#endif

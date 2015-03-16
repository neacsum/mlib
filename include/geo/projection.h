#pragma once
/*!
  \file PROJECTI.H	- Projection class definition

*/

#include "ellip.h"

#include <mlib/errorcode.h>
#include <assert.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

class ProjParams 
{
public:
  ProjParams (const Ellipsoid& ell);
  ProjParams (Ellipsoid::well_known wk = Ellipsoid::WGS_84);

  ProjParams& ellipsoid (const Ellipsoid& ell);
  ProjParams& ellipsoid (Ellipsoid::well_known wk);
  ProjParams& k0 (double k);
  ProjParams& unit (double u);
  ProjParams& ref_latitude (double phi);
  ProjParams& ref_longitude (double lambda);
  ProjParams& north_latitude (double phi);
  ProjParams& south_latitude (double phi);
  ProjParams& skew_azimuth (double alpha);
  ProjParams& false_north (double y);
  ProjParams& false_east (double x);

private:
  Ellipsoid ellip_;
  double k_;
  double unit_;
  double reflat_;
  double reflon_;
  double fn_;
  double fe_;
  double npar_;
  double spar_;
  double skew_;

  friend class Projection;
  friend class ConicalProjection;
  friend class ObliqueMercator;
};

class Projection
{
public:
  Projection ();
  Projection (const ProjParams& params);

  /// \name Parameter accessor functions
  ///\{
  double unit () const;
  double k0 () const;
  double ref_longitude () const;
  double ref_latitude () const;
  double false_east () const;
  double false_north () const;
  const Ellipsoid& ellipsoid () const;
  ///\}

  virtual errc xy_geo (double x, double y, double *lat, double *lon) const = 0;
  virtual errc geo_xy (double *x, double *y, double lat, double lon) const = 0;

  virtual double h (double lat, double lon) const = 0;
  virtual double k (double lat, double lon) const = 0;

protected:
  ProjParams par;
};

class ConicalProjection : public Projection
{
public:
  ConicalProjection ();
  ConicalProjection (const ProjParams& params);

  double north_latitude () const;
  double south_latitude () const;
};

///Error codes
#define GEOERR_PARAM    1       ///< Invalid projection parameters
#define GEOERR_SINGL    2       ///< Singularity
#define GEOERR_DOMAIN   3       ///< Domain error
#define GEOERR_NCONV    4       ///< Non-convergence


/*==================== INLINE FUNCTIONS ===========================*/

/// Set scale factor at origin
inline 
ProjParams& ProjParams::k0 (double k)
{
  k_ = k;
  return *this;
}

///Set conversion factor from XY units to meters
inline
ProjParams& ProjParams::unit (double u)
{
  unit_ = u;
  return *this;
}


/// Set reference latitude
inline
ProjParams& ProjParams::ref_latitude (double phi)
{
  assert (-M_PI / 2 <= phi && phi <= M_PI / 2);
  reflat_ = phi;
  return *this;
}

/// Set reference longitude (central meridian)
inline
ProjParams& ProjParams::ref_longitude (double lambda)
{
  assert (-M_PI <= lambda && lambda <= M_PI);
  reflon_ = lambda;
  return *this;
}


inline
ProjParams& ProjParams::north_latitude (double phi)
{
  assert (-M_PI / 2 <= phi && phi <= M_PI / 2);
  npar_ = phi;
  return *this;
}

inline
ProjParams& ProjParams::south_latitude (double phi)
{
  assert (-M_PI / 2 <= phi && phi <= M_PI / 2);
  spar_ = phi;
  return *this;
}

inline
ProjParams& ProjParams::skew_azimuth (double alpha)
{
  assert (-M_PI <= alpha && alpha <= M_PI);
  skew_ = alpha;
  return *this;
}

inline
ProjParams& ProjParams::false_east (double x)
{
  fe_ = x;
  return *this;
}

inline
ProjParams& ProjParams::false_north (double y)
{
  fn_ = y;
  return *this;
}

/// Return scale factor at origin
inline
double Projection::k0 () const { return par.k_; }

/// Return central meridian
inline
double Projection::ref_longitude () const { return par.reflon_; }

/// Return conversion factor from XY units to meters
inline
double Projection::unit () const { return par.unit_; }

/// Return reference latitude
inline
double Projection::ref_latitude () const { return par.reflat_; }

/// Return X (easting) value at origin
inline
double Projection::false_east () const { return par.fe_; }

/// Return Y (northing) value at origin
inline
double Projection::false_north () const { return par.fn_; }

/// Return projection's ellipsoid
inline
const Ellipsoid& Projection::ellipsoid () const { return par.ellip_; }

/// Return North parallel
inline
double ConicalProjection::north_latitude () const { return par.npar_; }

/// Return South parallel
inline
double ConicalProjection::south_latitude () const { return par.spar_; }


//========================= Helper functions ==================================

///  Return an adjusted longitude between -M_PI and M_PI.
double lon_adjust (double lon);

#ifdef MLIBSPACE
};
#endif

#pragma once
/*!
  \file PROJECTION.H - Projection class definition

*/

#include "ellip.h"

#include <mlib/errorcode.h>
#include <assert.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif


class Projection
{
public:
  class Params
  {
  public:
    Params (const Ellipsoid& ell);
    Params (Ellipsoid::well_known wk = Ellipsoid::WGS_84);

    Params& ellipsoid (const Ellipsoid& ell);
    Params& ellipsoid (Ellipsoid::well_known wk);
    Params& k0 (double k);
    Params& unit (double u);
    Params& ref_latitude (double phi);
    Params& ref_longitude (double lambda);
    Params& skew_azimuth (double alpha);
    Params& north_latitude (double phin);
    Params& south_latitude (double phis);
    Params& false_north (double y);
    Params& false_east (double x);

  private:
    Ellipsoid ellip_;
    double k_;
    double unit_;
    double reflat_;
    double reflon_;
    double fn_;
    double fe_;
    double skew_;
    double npar_;
    double spar_;

    friend class Projection;
    friend class ConicalProjection;
    friend class ObliqueMercator;
  };

  Projection ();
  Projection (const Params& params);

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
  Params par;
};

/// Common base for conical projections
class ConicalProjection : public Projection
{
public:
  ConicalProjection ();
  ConicalProjection (const Params& params);

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
Projection::Params& Projection::Params::k0 (double k)
{
  k_ = k;
  return *this;
}

///Set conversion factor from XY units to meters
inline
Projection::Params& Projection::Params::unit (double u)
{
  unit_ = u;
  return *this;
}


/// Set reference latitude
inline
Projection::Params& Projection::Params::ref_latitude (double phi)
{
  assert (-M_PI / 2 <= phi && phi <= M_PI / 2);
  reflat_ = phi;
  return *this;
}

/// Set reference longitude (central meridian)
inline
Projection::Params& Projection::Params::ref_longitude (double lambda)
{
  assert (-M_PI <= lambda && lambda <= M_PI);
  reflon_ = lambda;
  return *this;
}


inline
Projection::Params& Projection::Params::skew_azimuth (double alpha)
{
  assert (-M_PI <= alpha && alpha <= M_PI);
  skew_ = alpha;
  return *this;
}

inline
Projection::Params& Projection::Params::false_east (double x)
{
  fe_ = x;
  return *this;
}

inline
Projection::Params& Projection::Params::false_north (double y)
{
  fn_ = y;
  return *this;
}

inline
Projection::Params& Projection::Params::north_latitude (double phin)
{
  assert (-M_PI / 2 <= phin && phin <= M_PI / 2);
  npar_ = phin;
  return *this;
}

inline
Projection::Params& Projection::Params::south_latitude (double phis)
{
  assert (-M_PI / 2 <= phis && phis <= M_PI / 2);
  spar_ = phis;
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

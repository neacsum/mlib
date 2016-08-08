#pragma once

/*!
  \file STEREO.H - Definition of sterographic and Polar %Stereograpic projections  

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// %Stereographic projection
class Stereographic : public Projection
{
public:
  Stereographic () {};
  Stereographic (const Params& params);
  Stereographic& operator= (const Params& p);

  errc geo_xy (double *x, double *y, double lat, double lon) const;
  errc xy_geo (double x, double y, double *lat, double *lon) const;
  
  double k (double lat, double lon) const;
  double h (double lat, double lon) const;

private:
  void init ();
  double c1, c2, chi0, lam0s, r0;
  double map_radius;
};

/// Polar %Stereographic
class PolarStereo : public Projection
{
public:
  PolarStereo () {};
  PolarStereo (const Params& params);
  PolarStereo& operator= (const Params& p);

  errc geo_xy (double *x, double *y, double lat, double lon) const;
  errc xy_geo (double x, double y, double *lat, double *lon) const;
  double k (double lat, double lon) const;
  double h (double lat, double lon) const;

private:
  void init ();
  double rho (double lat) const;
  double rho1;
  double sc[4];
  double k_;
};

inline
double Stereographic::h (double lat, double lon) const { return k (lat, lon); }

inline
double PolarStereo::h (double lat, double lon) const { return k (lat, lon); }

#ifdef MLIBSPACE
};
#endif

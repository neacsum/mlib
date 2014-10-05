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
  Stereographic (PROJPARAMS& pp);
  const char *Name () const              { return "OST"; };
  geoproj Id () const                    { return GEOPROJ_OST; };
  errc GeoXY (double *x, double *y, double lat, double lon) const;
  errc XYGeo (double x, double y, double *lat, double *lon) const;
  double k (double lat, double lon) const;

private:
  double c1, c2, chi0, lam0s, r0;
  double map_radius;
};

/// Polar %Stereographic
class PolarStereo : public Projection
{
public:
  PolarStereo (PROJPARAMS& pp);
  const char *Name () const              { return "PST"; };
  geoproj Id () const                    { return GEOPROJ_PST; };
  errc GeoXY (double *x, double *y, double lat, double lon) const;
  errc XYGeo (double x, double y, double *lat, double *lon) const;
  double k (double lat, double lon) const;

private:
  double rho (double lat) const;
  double rho1;
  double sc[4];
  //	double map_radius;
};

#ifdef MLIBSPACE
};
#endif

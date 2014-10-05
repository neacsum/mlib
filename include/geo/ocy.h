#pragma once

/*!
  \file OCY.H - Oblique Cylindrical projection definition

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Oblique Cylindrical
class ObliqueCylindrical : public Projection
{
public:
  ObliqueCylindrical (PROJPARAMS& pp);
  errc XYGeo (double x, double y, double *lat, double *lon) const;
  errc GeoXY (double *x, double *y, double lat, double lon) const;

  double h (double lat, double lon) const   { return k (lat, lon); };
  double k (double lat, double lon) const;

  const char *Name () const                  { return "OCY"; };
  geoproj Id () const                        { return GEOPROJ_OCY; };
private:
  double map_radius, c1, c2, chi0;
};

#ifdef MLIBSPACE
};
#endif

#pragma once

/*!
  \file OCY.H - Oblique Cylindrical projection definition

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Oblique Cylindrical
/// \ingroup geo
class ObliqueCylindrical : public Projection
{
public:
  ObliqueCylindrical () {};
  ObliqueCylindrical (const Params& params);
  ObliqueCylindrical& operator= (const Params& p);

  errc xy_geo (double x, double y, double *lat, double *lon) const;
  errc geo_xy (double *x, double *y, double lat, double lon) const;

  double h (double lat, double lon) const   { return k (lat, lon); };
  double k (double lat, double lon) const;

private:
  void init ();
  double map_radius, c1, c2, chi0;
};

#ifdef MLIBSPACE
};
#endif

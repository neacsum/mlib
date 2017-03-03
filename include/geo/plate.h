#pragma once
/*!
  \file PLATE.H - Plate Carrée projection definition

*/

#include <geo/projection.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Plate Carrée projection
/// \ingroup geo
class PlateCarree : public Projection
{
public:
  PlateCarree (const Params& params);

  errc xy_geo (double x, double y, double *lat, double *lon) const;
  errc geo_xy (double *x, double *y, double lat, double lon) const;

  double h (double lat, double lon) const   { return 1.; };
  double k (double lat, double lon) const;
};


#ifdef MLIBSPACE
};
#endif

#pragma once
/*!
  \file PLATE.H - Plate Caree projection definition

*/

#include <geo/projection.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

///Plate Carree projection
class PlateCarree : public Projection
{
public:
  PlateCarree (PROJPARAMS& pp);
  errc XYGeo (double x, double y, double *lat, double *lon) const;
  errc GeoXY (double *x, double *y, double lat, double lon) const;

  double h (double lat, double lon) const   { return 1.; };
  double k (double lat, double lon) const;

  const char *Name () const     { return "PLC"; };
  geoproj Id () const           { return GEOPROJ_DEM; };
private:
  double sfeq;                  //radius of circle of parallel
};


#ifdef MLIBSPACE
};
#endif

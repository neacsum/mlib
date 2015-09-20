#pragma once

/*!
  \file MERCATOR.H - Mercator and \ref SMerc "CMAP Mercator" projections

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// %Mercator projection
class Mercator : public Projection
{
public:
  Mercator ();
  Mercator (const Params& params);
  Mercator& operator= (const Params& p);

  errc xy_geo (double x, double y, double *lat, double *lon) const;
  errc geo_xy (double *x, double *y, double lat, double lon) const;

  double h (double lat, double lon) const {return k(lat, lon);};
  double k (double lat, double lon) const;

private:
  void init ();
  double sfeq;                  ///< radius of circle of parallel
};


///CMap %Mercator
class CMapMercator : public Projection
{
public:
  CMapMercator ();
  errc xy_geo(double x, double y, double *lat, double *lon) const;
  errc geo_xy(double *x, double *y, double lat, double lon) const;
  double h (double lat, double lon) const {return k(lat, lon);};
  double k (double lat, double lon) const;
};


#ifdef MLIBSPACE
};
#endif


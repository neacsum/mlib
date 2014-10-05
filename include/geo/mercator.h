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
  Mercator(  PROJPARAMS& pp );
  errc XYGeo(double x, double y, double *lat, double *lon) const;
  errc GeoXY(double *x, double *y, double lat, double lon) const;

  double h (double lat, double lon) const {return k(lat, lon);};
  double k (double lat, double lon) const;

  const char *Name() const      { return "MER"; };
  geoproj Id() const            { return GEOPROJ_MER; };

private:
  double sfeq;                  ///< radius of circle of parallel
};


///CMap %Mercator
class SMerc : public Projection
{
public:
  SMerc (PROJPARAMS& pp);
  errc XYGeo(double x, double y, double *lat, double *lon) const;
  errc GeoXY(double *x, double *y, double lat, double lon) const;
  double h (double lat, double lon) const {return k(lat, lon);};
  double k (double lat, double lon) const;
  const char *Name() const      { return "CME"; };
  geoproj Id() const            { return GEOPROJ_CME; };
};


#ifdef MLIBSPACE
};
#endif


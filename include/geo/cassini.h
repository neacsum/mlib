#pragma once

/*!
  \file CASSINI.H - Cassini projection definition

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Cassini-Soldner projection
class Cassini : public Projection
{
 public:
  Cassini (PROJPARAMS& pp);
  const char *Name() const      { return "CAS"; };
  geoproj Id() const            { return GEOPROJ_CAS; };

  errc XYGeo (double x, double y, double *lat, double *lon) const;
  errc GeoXY (double *x, double *y, double lat, double lon) const ;
  double h (double lat, double lon) const;
  double k (double lat, double lon) const {return 1.;};

 private:
   double s0;
};

#ifdef MLIBSPACE
};
#endif

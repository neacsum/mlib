#pragma once

/*!
  \file CAS.H - Cassini projection definition

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Cassini-Soldner projection
class Cassini : public Projection
{
 public:
  Cassini () {};
  Cassini (const Params& params);

  Cassini& operator= (const Params& p);

  errc xy_geo (double x, double y, double *lat, double *lon) const;
  errc geo_xy (double *x, double *y, double lat, double lon) const ;
  double h (double lat, double lon) const;
  double k (double lat, double lon) const {return 1.;};

 private:
   void init ();
   double s0;
};

#ifdef MLIBSPACE
};
#endif

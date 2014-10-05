#pragma once

/*!
  \file OME.H - ObliqueMercator class definition

*/

#include "projection.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

///Oblique %Mercator
class ObliqueMercator : public Projection
{
public:
  ObliqueMercator (PROJPARAMS& pp);
  errc GeoXY( double *x, double *y, double lat, double lon ) const;
  errc XYGeo( double x, double y, double *lat, double *lon ) const;

  const char *Name () const      { return "OME"; };
  geoproj Id () const            { return GEOPROJ_OME; };

  double h (double lat, double lon) const   { return k (lat, lon); };
  double k (double lat, double lon) const;

protected:
  virtual void deskew( double u, double v, double *x, double *y ) const = 0;
  virtual void skew( double *u, double *v, double x, double y ) const = 0;
  double exptau (double val) const;
  double gama0;

private:
  errc uvgeo (double u, double v, double *lat, double *lon) const;
  double A, B, E, lam1;
};

#ifdef MLIBSPACE
};
#endif

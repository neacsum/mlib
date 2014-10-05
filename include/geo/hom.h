#pragma once

/*!
  \file HOM.H - \ref Hotine "Hotine Oblique Mercator" definition

*/

#include "ome.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// %Hotine Oblique %Mercator
class Hotine : public ObliqueMercator
{
public:
  Hotine (PROJPARAMS& pp) : ObliqueMercator (pp) {};
  const char *Name() const      { return "HOM"; };
  geoproj Id() const            { return GEOPROJ_HOM; };

protected:
  void deskew( double u, double v, double *x, double *y ) const;
  void skew( double *u, double *v, double x, double y ) const;

};

#ifdef MLIBSPACE
};
#endif

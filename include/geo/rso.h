#pragma once

/*!
  \file RSO.H - Rectified Skew Orthomorphic projection definition

*/

#include "ome.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Rectified Skew Orthomorphic
class RSO : public ObliqueMercator
{
public:
  RSO (PROJPARAMS& pp) : ObliqueMercator (pp) {};
  const char *Name () const   { return "RSO"; };
  geoproj Id () const  { return GEOPROJ_RSO; };

protected:
  void deskew (double u, double v, double *x, double *y) const;
  void skew (double *u, double *v, double x, double y) const;
};

#ifdef MLIBSPACE
};
#endif

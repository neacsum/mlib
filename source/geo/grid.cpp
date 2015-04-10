/*!
  \file GRID.CPP - Implementation of Gridded class
*/

#include <geo/grid.h>
#include <geo/convert.h>

#include <mlib/utf8.h>
#include <mlib/mathval.h>

#include <math.h>
#include <memory.h>
#include <string.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Default error facility for griding errors
errfacility errors("Grid Error");

/// Indirection pointer allows user to redirect errors to another facility
errfacility *grid_errors = &errors;

#define MISSING_MIN   1
#define MISSING_MAX   2
#define MISSING_DELTA 4
#define MISSING_COUNT 8

GridParams::GridParams () :
x0_ (0), x1_ (0), dx_ (0), nc_ (0),
y0_ (0), y1_ (0), dy_ (0), nr_ (0),
sz_ (Gridded::sz_float),
m_ (Gridded::meth_bilin),
s_ (Gridded::planar),
nz_ (1),
x_constraints (0), y_constraints (0),
x_defs (0x0f), 
y_defs (0x0f),
noval_ (9E99)
{
}

GridParams& GridParams::xmin (double v)
{
  if (x_constraints >= 3)
    throw errc (ERR_GRID_CONSTRAINTS, ERROR_PRI_ERROR, grid_errors);
  x0_ = v;
  x_defs &= ~MISSING_MIN;
  if (++x_constraints == 3)
    recalc ();

  return *this;
}

GridParams& GridParams::xmax (double v)
{
  if (x_constraints >= 3)
    throw errc (ERR_GRID_CONSTRAINTS, ERROR_PRI_ERROR, grid_errors);
  x1_ = v;
  x_defs &= ~MISSING_MAX;
  if (++x_constraints == 3)
    recalc ();

  return *this;
}

GridParams& GridParams::dx (double v)
{
  if (x_constraints >= 3)
    throw errc (ERR_GRID_CONSTRAINTS, ERROR_PRI_ERROR, grid_errors);
  dx_ = v;
  x_defs &= ~MISSING_DELTA;
  if (++x_constraints == 3)
    recalc ();

  return *this;
}

GridParams& GridParams::columns (int c)
{
  if (x_constraints >= 3)
    throw errc (ERR_GRID_CONSTRAINTS, ERROR_PRI_ERROR, grid_errors);
  nc_ = c;
  x_defs &= ~MISSING_COUNT;
  if (++x_constraints == 3)
    recalc ();

  return *this;
}

GridParams& GridParams::ymin (double v)
{
  if (y_constraints >= 3)
    throw errc (ERR_GRID_CONSTRAINTS, ERROR_PRI_ERROR, grid_errors);
  y0_ = v;
  y_defs &= ~MISSING_MIN;
  if (++y_constraints == 3)
    recalc ();

  return *this;
}

GridParams& GridParams::ymax (double v)
{
  if (y_constraints >= 3)
    throw errc (ERR_GRID_CONSTRAINTS, ERROR_PRI_ERROR, grid_errors);
  y1_ = v;
  y_defs &= ~MISSING_MAX;
  if (++y_constraints == 3)
    recalc ();

  return *this;
}

GridParams& GridParams::dy (double v)
{
  if (y_constraints >= 3)
    throw errc (ERR_GRID_CONSTRAINTS, ERROR_PRI_ERROR, grid_errors);
  dy_ = v;
  y_defs &= ~MISSING_DELTA;
  if (++y_constraints == 3)
    recalc ();

  return *this;
}

GridParams& GridParams::rows (int r)
{
  if (y_constraints >= 3)
    throw erc (ERR_GRID_CONSTRAINTS, ERROR_PRI_ERROR, grid_errors);
  nr_ = r;
  y_defs &= ~MISSING_COUNT;
  if (++y_constraints == 3)
    recalc ();

  return *this;
}

/*!
  Calculate missing grid parameters based on existing ones
*/
void GridParams::recalc ()
{
  if (x_constraints == 3)
  {
    switch (x_defs)
    {
    case MISSING_COUNT:
      nc_ = (int)(abs ((x1_ - x0_)) / dx_);
      x_defs &= ~MISSING_COUNT;
      break;

    case MISSING_DELTA:
      dx_ = abs (x1_ - x0_) / nc_;
      x_defs &= ~MISSING_DELTA;
      break;

    case MISSING_MAX:
      x1_ = x0_ + nc_*dx_;
      x_defs &= ~MISSING_MAX;
      break;
      
    case MISSING_MIN:
      x0_ = x1_ - nc_*dx_;
      x_defs &= ~MISSING_MIN;
      break;
    }
  }
  if (y_constraints == 3)
  {
    switch (y_defs)
    {
    case MISSING_COUNT:
      nr_ = (int)(abs ((y1_ - y0_)) / dy_);
      y_defs &= ~MISSING_COUNT;
      break;

    case MISSING_DELTA:
      dy_ = abs (y1_ - y0_) / nr_;
      y_defs &= ~MISSING_DELTA;
      break;

    case MISSING_MAX:
      y1_ = y0_ + nr_*dy_;
      y_defs &= ~MISSING_MAX;
      break;

    case MISSING_MIN:
      y0_ = y1_ - nr_*dy_;
      y_defs &= ~MISSING_MIN;
      break;
    }
  }
}

Gridded::Gridded () :
x0_ (0), x1_ (0), dx_ (0), nc_ (0),
y0_ (0), y1_ (0), dy_ (0), nr_ (0),
sz_ (sz_float),
method_ (meth_bilin),
space_ (planar),
nz_ (1),
noval_ (9E99)
{
}

Gridded::Gridded (const GridParams& p)
{
  set_params (p);
}


Gridded::~Gridded ()
{
}

erc Gridded::set_params (const GridParams& p)
{
  if (p.x_constraints != 3 || p.y_constraints != 3)
    return erc (ERR_GRID_UNCONSTRAINT, ERROR_PRI_ERROR, grid_errors);
  x0_ = p.x0_; x1_ = p.x1_; dx_ = p.dx_;
  y0_ = p.y0_; y1_ = p.y1_; dy_ = p.dy_;
  nr_ = p.nr_; nc_ = p.nc_; nz_ = p.nz_;
  sz_ = p.sz_;
  method_ = p.m_;
  space_ = p.s_;
  noval_ = p.noval_;
  return ERR_SUCCESS;
}


/*
  Weighting function
*/
static double msl_func (double x, double y)
{
  // MSL algorithm weighting
  return x*x*y*y*(9. - 6.*x - 6.*y + 4.*x*y);
}

/*!
    Calculate an interpolated value given the four corners of an
    interpolation cell and reduced coordinates of the interpolation point.

    Cell corners are arranged in the (counter-clockwise) order:
    -  v[0] - x=0, y=0
    -  v[1] - x=1, y=0
    -  v[2] - x=1, y=1
    -  v[3] - x=0, y=1
*/
double Gridded::msl (const double *v, double x, double y) const
{
  double w[4];      //weights
  double result;    //interpolated value

  //calculate weight vector
  w[0] = msl_func (1 - x, 1 - y);
  w[1] = msl_func (x, 1 - y);
  w[2] = msl_func (x, y);
  w[3] = msl_func (1 - x, y);

  result = 0.;
  for (int i = 0; i < 4; i++)
    result += w[i] * v[i];
  return result;
}

double Gridded::bilinear (const double *v, double x, double y) const
{
  double w[4];      //weights
  double result;    //interpolated value

  //calculate weight vector
  w[0] = (1 - x) * (1 - y);
  w[1] = x * (1 - y);
  w[2] = x * y;
  w[3] = (1 - x) * y;

  result = 0.;
  for (int i = 0; i < 4; i++)
    result += w[i] * v[i];
  return result;
}

static double qterp2 (double x, double f0, double f1, double f2)
{
  double df0 = f1 - f0,
    df1 = f2 - f1,
    d2f0 = df1 - df0;
  return f0 + x*df0 + 0.5*x*(x - 1)*d2f0;
}

double Gridded::biquad (const double *v, double x, double y) const
{
  double fx0 = qterp2 (x, v[0], v[1], v[2]); //lower boundary
  double fx1 = qterp2 (x, v[3], v[4], v[5]); //middle row
  double fx2 = qterp2 (x, v[6], v[7], v[8]); //upper boundary
  return qterp2 (y, fx0, fx1, fx2);
}


#ifdef MLIBSPACE
};
#endif

#pragma once
/*!
  \file GRID.H -  Grid class definition
*/

#include <mlib/errorcode.h>
#include <string>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

extern errfacility *grid_errors;

#define ERR_GRID_CONSTRAINTS    1   ///< too many constraints
#define ERR_GRID_UNCONSTRAINT   2   ///< not enough constraints
#define ERR_GRID_FILEOPEN       3   ///< failed to open file
#define ERR_GRID_FILEREAD       4   ///< file read error
#define ERR_GRID_OUTSIDE        5   ///< outside grid limits

class GridParams;

/*!
  \class Gridded - Gridded data used for 2D interpolation
  
  This is an abstract class that captures common properties between memory
  and file based data. The data can be gridded either on a latitude/longitude
  or a planar grid.

  The main function, Interpolate, finds the grid square where the interpolated
  point lies and calls the required interpolation function to do the interpolation 
  between the 4 corners of the grid.

  The interpolation method can be one of the following:
  - bilinear interpolation
  - spline (cubic spline)
  - quadratic
  - biquadratic
  - DMA MSL algorithm

  A grid can contain more than one data plane. Each plane is interpolated
  independently of the other data planes.

*/
class Gridded
{
public:
  enum datasz { sz_byte, sz_short, sz_int, sz_float, sz_double }; ///< size of data
  enum method { meth_bilin, meth_spline, meth_biquad, meth_msl }; ///< interpolation method
  enum space { planar, cylindrical, spherical };    ///< interpolation space


  //constructors and destructor
  Gridded ();
  Gridded (const GridParams& params);
  virtual ~Gridded ();

  //public functions
  erc         set_params (const GridParams& p);
  void        limits (double& xmin, double& xmax, double& ymin, double& ymax) const;
  void        resolution (double& xres, double& yres) const;
  int         rows () const;
  int         cols () const;
  int         z_count () const;
  datasz      data_size () const;
  method      interp_method () const;
  double      no_data () const;
  virtual erc Interpolate (double x, double y, double *interp) = 0;

  //Interpolation functions
  double bilinear (const double *v, double x, double y) const;
  double spline (const double *v, double x, double y);
  double quad (const double *v, double y);
  double biquad (const double *v, double x, double y) const;
  double msl (const double *v, double x, double y) const;

protected:

  double x0_, x1_, y0_, y1_, dx_, dy_;
  int nr_, nc_, nz_;
  datasz sz_;
  method method_;
  space space_;
  double noval_;
};

/*!
  Support class containing the parameters of an interpolation grid.

  It can be used with the "named parameter" idiom to set any combination of
  grid parameters. Grid extent can be specified either by the number of cells
  or by extent and cell spacing. This class ensures that missing parameters
  are set to appropriate values.
*/
class GridParams {
public:
  GridParams ();
  GridParams& xmin (double v);
  GridParams& xmax (double v);
  GridParams& columns (int c);
  GridParams& dx (double v);

  GridParams& ymin (double v);
  GridParams& ymax (double v);
  GridParams& rows (int r);
  GridParams& dy (double v);

  GridParams& planes (int p);
  GridParams& data_size (Gridded::datasz sz);
  GridParams& interp_method (Gridded::method m);
  GridParams& interp_space (Gridded::space s);
  GridParams& empty_value (double v);

private:
  void recalc ();
  double x0_, x1_, dx_;
  double y0_, y1_, dy_;
  int nc_, nr_;
  unsigned char x_constraints, y_constraints;
  unsigned char x_defs, y_defs;

  int nz_;
  Gridded::datasz sz_;
  Gridded::method m_;
  Gridded::space s_;
  double noval_;

  friend class Gridded;
};


inline GridParams& GridParams::planes (int p) { nz_ = p; return *this; }
inline GridParams& GridParams::data_size (Gridded::datasz sz) { sz_ = sz; return *this; }
inline GridParams& GridParams::interp_method (Gridded::method m) { m_ = m; return *this; }
inline GridParams& GridParams::interp_space (Gridded::space s) { s_ = s; return *this; }
inline GridParams& GridParams::empty_value (double v) { noval_ = v; return *this; }

inline int Gridded::rows () const { return nr_; }
inline int Gridded::cols () const { return nc_; }
inline int Gridded::z_count () const { return nz_; }
inline double Gridded::no_data () const { return noval_; }


inline 
void Gridded::limits (double& xmin, double& xmax, double& ymin, double& ymax) const
{
  xmin = x0_; xmax = x1_; ymin = y0_; ymax = y1_;
}

inline
void Gridded::resolution (double& xres, double& yres) const
{
  xres = dx_; yres = dy_;
}


#ifdef MLIBSPACE
};
#endif

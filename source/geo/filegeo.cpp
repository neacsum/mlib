/*!
\file filegeo.cpp - Implementation of FileGEO
*/

#include <geo/filegeo.h>
#include <geo/convert.h>
#include <mlib/mathval.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif


FileGEO::FileGEO (const char *filename, bool west_positive) :
rec1 (NULL),
rec2 (NULL)
{
  if (!(file = fopen (filename, "rb")))
    throw erc (ERR_GRID_FILEOPEN, ERROR_PRI_ERROR, grid_errors);

  if (fread (&header, sizeof (geo_header), 1, file) != 1)
    throw erc (ERR_GRID_FILEREAD, ERROR_PRI_ERROR, grid_errors);

  GridParams p;
  p.xmin (header.x01*D2R)
    .ymin (header.y01*D2R)
    .dx ((header.dx1*3600. + 0.5) / 3600.*M_PI / 180.)
    .dy ((header.dy1*3600. + 0.5) / 3600.*M_PI / 180.)
    .rows ((int)header.nr)
    .columns ((int)header.nc)
    .planes ((int)header.nz)
    .interp_space (Gridded::spherical);

  set_params (p);
/*
  if (west_positive)
  {
    //change sign and swap longitude limits
    double temp = -x1;
    x1 = -x0;
    x0 = temp;
  }
  if (y0 < -M_PI / 2 || y1 > M_PI / 2 || x0 < -M_PI || x1 > M_PI)
  throw erc (IFERR_READ);
*/
  recl = sizeof (float)*(nz_*nc_ + 1);
  rec1 = new float[nc_*nz_ + 1];
  rec2 = new float[nc_*nz_ + 1];
  ilat = ilon = -1;     //v vector will be updated on next call to Interpolate
}

FileGEO::~FileGEO ()
{
  delete[]rec1;
  delete[]rec2;
  if (file)
    fclose (file);
}

/*!
Calculate interpolated values.

If interpolation point is outside file limits return FINTERP_ERR_OUTOFLIMITS.
After reducing the point to unit square calls UnitInterp() to do the actual
interpolation.
*/
errc FileGEO::Interpolate (double lat, double lon, double *interp)
{
  double v[4];
  erc retcode;

  if (y1_ > M_PI && lon < 0)
    lon = 2 * M_PI + lon;     //adjust for transition from eastern hemisphere to western

  if (lat > y1_ || lat < y0_ || lon > x1_ || lon < x0_)
    return erc (ERR_GRID_OUTSIDE, ERROR_PRI_ERROR, grid_errors);

  int ilat_new, ilon_new;     //new indexes in grid

  ilat_new = (int)((lat - y0_) / dy_);
  ilon_new = (int)((lon - x0_) / dx_);

  if (ilat_new != ilat || ilon_new != ilon)
  {
    //need to read new cell corner values
    ilat = ilat_new;
    ilon = ilon_new;
    fseek (file, (ilat + 1)*recl, SEEK_SET); //1 is for header record

    //read record with "lower" corners of the cell
    fread (rec1, sizeof (float), nc_*nz_ + 1, file);
    //and record with "upper" corners
    fread (rec2, sizeof (float), nc_*nz_ + 1, file);
  }


  //reduce lat/lon to grid square
  double y = (lat - (y0_ + ilat*dy_)) / dy_;
  double x = (lon - (x0_ + ilon*dx_)) / dx_;

  for (int i = 0; i < nz_; i++)
  {
    v[0] = rec1[ilon*nz_ + i + 1]; //records have a dummy float at the beginning
    v[1] = rec1[(ilon + 1)*nz_ + i + 1];
    v[2] = rec2[(ilon + 1)*nz_ + i + 1];
    v[3] = rec2[ilon*nz_ + i + 1];
    *interp++ = bilinear (v, x, y);
  }
  return ERR_SUCCESS;
}



#ifdef MLIBSPACE
};
#endif

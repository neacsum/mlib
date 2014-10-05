/*!
  \file PLATE.CPP - Plate Caree implementation

*/

#include <geo/plate.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

PlateCarree::PlateCarree (PROJPARAMS& pp) :
  Projection (pp)
{
}

errc PlateCarree::XYGeo (double x, double y, double *lat, double *lon) const
{
  double xtrue = x - false_east_;
  double ytrue = y - false_north_;
  *lat = ytrue / a ();
  *lon = central_meridian_ + xtrue / (a ()*cos (ref_latitude_));
  return ERR_SUCCESS;
}

errc PlateCarree::GeoXY (double *x, double *y, double lat, double lon) const
{
  lon = LonAdjust (lon - central_meridian_);
  *x = lon * a () * cos (ref_latitude_) + false_east_;
  *y = lat * a () + false_north_;
  return ERR_SUCCESS;
}

double PlateCarree::k (double lat, double lon) const
{
  return cos (ref_latitude_) / cos (lat);
}

#ifdef MLIBSPACE
};
#endif

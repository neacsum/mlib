/*!
  \file PLATE.CPP - Plate Carrée implementation

*/

#include <geo/plate.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

PlateCarree::PlateCarree (const Params& params) :
  Projection (params)
{
}

errc PlateCarree::xy_geo (double x, double y, double *lat, double *lon) const
{
  double xtrue = x - false_east();
  double ytrue = y - false_north();
  *lat = ytrue / ellipsoid().a ();
  *lon = ref_longitude() + xtrue / (ellipsoid().a ()*cos (ref_latitude()));
  return ERR_SUCCESS;
}

errc PlateCarree::geo_xy (double *x, double *y, double lat, double lon) const
{
  lon = lon_adjust (lon - ref_longitude());
  *x = lon * ellipsoid().a () * cos (ref_latitude()) + false_east();
  *y = lat * ellipsoid().a () + false_north();
  return ERR_SUCCESS;
}

double PlateCarree::k (double lat, double lon) const
{
  return cos (ref_latitude()) / cos (lat);
}

#ifdef MLIBSPACE
};
#endif

#include <utpp/utpp.h>

#include <geo/projection.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

class TestProjection : public Projection
{
public:
  TestProjection (PROJPARAMS& gp) : Projection (gp) {};
  errc GeoXY (double *x, double *y, double lat, double lon) const {*x = lon, *y = lat; return ERROR_SUCCESS;};
  errc XYGeo (double x, double y, double *lat, double *lon) const {*lon = x; *lat = y; return ERROR_SUCCESS;};
  geoproj Id () const {return GEOPROJ_DEM;};
  const char *Name () const { return "TestProjection";}; 
  Projection::LonAdjust;
};

PROJPARAMS gp = {
  1, 2, GEOPROJ_DEM, 4, 5, 6, 7, 8, 9, 10, 11, 12};

TEST (ProjectionParameters)
{

  TestProjection t (gp);
  CHECK_EQUAL (1, t.a ());
  CHECK_EQUAL (2, 1/t.f ());
  CHECK_EQUAL (4, t.Unit ());
  CHECK_EQUAL (5, t.ScaleFactor ());
  CHECK_EQUAL (6, t.CentralMeridian ());
  CHECK_EQUAL (7, t.ReferenceLatitude ());
  CHECK_EQUAL (8, t.NorthParallel ());
  CHECK_EQUAL (9, t.SouthParallel ());
  CHECK_EQUAL (10, t.SkewAzimuth ());
  CHECK_EQUAL (11, t.FalseEast ());
  CHECK_EQUAL (12, t.FalseNorth ());
}

/* This function was used for many years. */
double OldLonAdjust( double lon )
{
  int sign = (lon>=0)?1:-1;

  while ( fabs(lon) > M_PI )
    lon -= 2*M_PI * sign;
  return lon;
}

TEST (Projection_LongitudeAdjustment)
{
  TestProjection t(gp);

  CHECK_CLOSE (0.5, t.LonAdjust (0.5+4*M_PI), 1E-15);

  double fval = 0.5+4*M_PI;

  CHECK_CLOSE (OldLonAdjust(fval), t.LonAdjust (fval), 1e-15);
  CHECK_CLOSE (OldLonAdjust(-fval), t.LonAdjust (-fval), 1e-15);

}
#include <utpp/utpp.h>

#include <geo/ellip.h>
#include <geo/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

Ellipsoid clarke66 (6378206.4, 1/294.978698200);

TEST (Ellip_SemiminorAxis)
{
  //value from TR8350.2
  CHECK_CLOSE (6356752.3142, Ellipsoid::WGS84.b (), 1E-4);
}

TEST (Ellip_FirstEccentricity)
{
  //value from TR8350.2
  CHECK_CLOSE (8.1819190842622e-2, Ellipsoid::WGS84.e (), 1E-14);
}

TEST (Ellip_FirstEccentricitySquared)
{
  //value from TR8350.2
  CHECK_CLOSE (6.69437999014e-3, Ellipsoid::WGS84.e2 (), 1E-14);
}

/* Check WGS84 constants from CONVERT.H against those in ELLIP.H*/
TEST (Ellip_convert_h)
{
  Ellipsoid wgs1 (A_WGS84, 1/F1_WGS84);
  CHECK_CLOSE (wgs1.a (), Ellipsoid::WGS84.a (), 1e-3);
  CHECK_CLOSE (wgs1.f (), Ellipsoid::WGS84.f (), 1e-14);
}

TEST (Ellip_qAux)
{
  CHECK_CLOSE (1.2792602, clarke66.q(40*D2R), 1e-7);
}

TEST (Ellip_AuthaliticLatitude)
{
  double q = clarke66.q(40*D2R);
  double qp = clarke66.q(M_PI/2);

  CHECK_CLOSE (1.9954814, qp, 1e-7);
  CHECK_CLOSE (39.8722878, (clarke66.beta (40*D2R))/D2R, 1e-7);
}



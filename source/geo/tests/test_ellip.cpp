#include <utpp/utpp.h>

#include <geo/ellip.h>
#include <geo/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

TEST (Ellip_SemiminorAxis)
{
  //value from TR8350.2
  CHECK_CLOSE (6356752.3142, Ellipsoid(Ellipsoid::WGS_84).b (), 1E-4);
}

TEST (Ellip_FirstEccentricity)
{
  //value from TR8350.2
  CHECK_CLOSE (8.1819190842622e-2, Ellipsoid (Ellipsoid::WGS_84).e (), 1E-14);
}

TEST (Ellip_FirstEccentricitySquared)
{
  //value from TR8350.2
  CHECK_CLOSE (6.69437999014e-3, Ellipsoid (Ellipsoid::WGS_84).e2 (), 1E-14);
}

/* Default ellipsoid is WGS84*/
TEST (Ellip_DefaultEllipsoid)
{
  Ellipsoid wgs;
  CHECK_CLOSE (A_WGS84, wgs.a (), 1e-3);
  CHECK_CLOSE (F_WGS84, wgs.f (), 1e-14);
  CHECK_EQUAL ("WGS-84", wgs.name ());
}

TEST (Ellip_qAux)
{
  CHECK_CLOSE (1.2792602, Ellipsoid (Ellipsoid::CLARKE_1866).q (40 * D2R), 1e-7);
}

TEST (Ellip_AuthaliticLatitude)
{
  Ellipsoid ell (Ellipsoid::CLARKE_1866);
  double q = ell.q (40 * D2R);
  double qp = ell.q (M_PI / 2);

  CHECK_CLOSE (1.9954814, qp, 1e-7);
  CHECK_CLOSE (39.8722878, ell.beta (40 * D2R) / D2R, 1e-7);
}



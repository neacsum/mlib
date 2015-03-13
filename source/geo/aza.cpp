/*!
  \file AzimuthalEqualArea.CPP - Implementation of Azimuthal Equal Area projection

  Formulas from Snyder pag. 187-190

*/
#include <geo/aza.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

#define MAX_ITER  10    //max number of iterations in reverse formulas

static double sqrarg;
#define SQR(a) ((sqrarg = (a)) == 0.0 ? 0.0 : sqrarg * sqrarg)

/*!
  Constructor for a Azimuthal Equal Area projection
*/
AzimuthalEqualArea::AzimuthalEqualArea ()
{
}

AzimuthalEqualArea::AzimuthalEqualArea (const ProjParams& params) :
  Projection (params)
{
  init ();
}

AzimuthalEqualArea& AzimuthalEqualArea::operator= (const ProjParams& p)
{
  par = p;
  init ();
  return *this;
}

void AzimuthalEqualArea::init ()
{
  q_p = ellipsoid().q (M_PI / 2);
  r_q = ellipsoid().a ()*unit()*sqrt (q_p / 2);
  q_1 = ellipsoid().q (ref_latitude());
  m_1 = ellipsoid().m (ref_latitude());
  polar = (M_PI_2 - fabs (ref_latitude()) < 1.E-6);
  if (!polar)
  {
    beta_1 = ellipsoid().beta (ref_latitude());
    d = m_1 / (sqrt (q_p / 2) * cos (beta_1));
  }
  ncval = 1 - (1 - ellipsoid().e2 ())*ellipsoid().t (M_PI_2);
}

/*!
  Forward conversion from latitude/longitude to XY
*/
errc AzimuthalEqualArea::geo_xy (double *x, double *y, double lat, double lon) const
{
  double lonr = lon - ref_longitude ();
  if (!polar)
  {
    double blat = ellipsoid().beta(lat);
    double b = r_q*M_SQRT2/sqrt(1 + sin(beta_1)*sin(blat)
                                  + cos(beta_1)*cos(blat)*cos(lonr));
    *x = b*d*cos(blat)*sin(lonr);
    *y = b/d*(cos(beta_1)*sin(blat) - sin(beta_1)*cos(blat)*cos(lonr));
  }
  else
  {
    double rho = ellipsoid().a()*sqrt (q_p-ellipsoid().q(lat));
    *x = rho * sin(lonr);
    *y = ((ref_latitude()<0)?rho:-rho) * cos (lonr);
  }
  *x += false_east();
  *y += false_north();

  return ERR_SUCCESS;
}

/*!
  Inverse conversion from XY to geographical coordinates.
*/
errc AzimuthalEqualArea::xy_geo (double x, double y, double *lat, double *lon) const
{
  double rho, q;
  x -= false_east();
  y -= false_north();
  if (!polar)
  {
    rho = sqrt(x*x/(d*d)+d*d*y*y);
    double c_e = 2*asin(rho/(2*r_q));
    q = q_p*(cos(c_e)*sin(beta_1)+d*y*sin(c_e)*cos(beta_1)/rho);
    *lon = ref_longitude() + 
      atan2( x*sin(c_e),d*rho*cos(beta_1)*cos(c_e)-d*d*y*sin(beta_1)*sin(c_e));
  }
  else
  {
    rho = hypot(x, y);
    q = q_p - SQR(rho/ellipsoid().a());
    if (ref_latitude() < 0)
    {
      q = -q;
      *lon = ref_longitude()+ atan2(x, y);
    }
    else
      *lon = ref_longitude() + atan2(x, -y);
  }

  double phi;
  if (fabs((fabs(q) - ncval)) < 1e-7)
    phi = (q<0)?-M_PI_2:M_PI_2;
  else
  {
    double phi_new = asin(q/2);
    int iter=0;
    do
    {
      phi = phi_new;
      double sphi = sin(phi);
      phi_new = phi + SQR((1-ellipsoid().e2()*ipow(sphi, 3)))/(2*cos(phi))*
        (q/(1-ellipsoid().e2())-sphi/(1-ellipsoid().e2()*sphi*sphi)+ellipsoid().t(phi));
    } while ( fabs(phi_new-phi) > 1e-7 && iter < MAX_ITER);
    *lat = phi;
    if (iter >= MAX_ITER)
      return errc (GEOERR_NCONV);
  }
  *lat = phi;
  return ERR_SUCCESS;
}

double AzimuthalEqualArea::h (double lat, double lon) const
{
  return 1/k(lat, lon);
}

double AzimuthalEqualArea::k (double lat, double lon) const
{
  return sqrt (q_p-ellipsoid().q(lat))/ellipsoid().m(lat);
}

#ifdef MLIBSPACE
};
#endif


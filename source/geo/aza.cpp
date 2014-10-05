/*!
  \file AZA.CPP - Implementation of Azimuthal Equal Area projection

  Formulas from Snyder pag. 187-190

*/
#include <geo/aza.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

#define MAX_ITER  10    //max number of iterations in reverse formulas

/*!
  Constructor for a Azimuthal Equal Area projection
*/
AZA::AZA (PROJPARAMS& pp):
	Projection (pp)
{
  q_p = q(M_PI/2);
  r_q = a()*unit_*sqrt (q_p/2);
  q_1 = q(ref_latitude_);
  m_1 = m(ref_latitude_);
  polar = (M_PI_2 - fabs (ref_latitude_) < 1.E-6);
  if (!polar)
  {
    beta_1 = beta(ref_latitude_);
    d = m_1 / (sqrt (q_p/2) * cos(beta_1));
  }
  ncval = 1 - (1-e2())*t(M_PI_2);
}

/*!
  Forward conversion from latitude/longitude to XY
*/
errc AZA::GeoXY (double *x, double *y, double lat, double lon) const
{
  if (!polar)
  {
    double blat = beta(lat);
    double b = r_q*M_SQRT2/sqrt(1 + sin(beta_1)*sin(blat)
                                  + cos(beta_1)*cos(blat)*cos(lon-central_meridian_));
    *x = b*d*cos(blat)*sin(lon-central_meridian_);
    *y = b/d*(cos(beta_1)*sin(blat) - sin(beta_1)*cos(blat)*cos(lon-central_meridian_));
  }
  else
  {
    double rho = a()*sqrt (q_p-q(lat));
    *x = rho * sin(lon - central_meridian_);
    *y = ((ref_latitude_<0)?rho:-rho) * cos (lon - central_meridian_);
  }
  *x += false_east_;
  *y += false_north_;
  return ERR_SUCCESS;
}

/*!
  Inverse conversion from XY to geographical coordinates.
*/
errc AZA::XYGeo (double x, double y, double *lat, double *lon) const
{
  double rho, q;
  x -= false_east_;
  y -= false_north_;
  if (!polar)
  {
    rho = sqrt(x*x/(d*d)+d*d*y*y);
    double c_e = 2*asin(rho/(2*r_q));
    q = q_p*(cos(c_e)*sin(beta_1)+d*y*sin(c_e)*cos(beta_1)/rho);
    *lon = central_meridian_ + 
      atan2( x*sin(c_e),d*rho*cos(beta_1)*cos(c_e)-d*d*y*sin(beta_1)*sin(c_e));
  }
  else
  {
    rho = hypot(x, y);
    q = q_p - rho*rho/(a()*a());
    if (ref_latitude_ < 0)
    {
      q = -q;
      *lon = central_meridian_ + atan2(x, y);
    }
    else
      *lon = central_meridian_ + atan2(x, -y);
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
      phi_new = phi + ipow((1-e2()*ipow(sphi, 3)),2)/(2*cos(phi))*
        (q/(1-e2())-sphi/(1-e2()*sphi*sphi)+t(phi));
    } while ( fabs(phi_new-phi) > 1e-7 && iter < MAX_ITER);
    *lat = phi;
    if (iter >= MAX_ITER)
      return errc (GEOERR_NCONV);
  }
  *lat = phi;
  return ERR_SUCCESS;
}

double AZA::h (double lat, double lon) const
{
  return 1/k(lat, lon);
}

double AZA::k (double lat, double lon) const
{
  return sqrt (q_p-q(lat))/m(lat);
}

#ifdef MLIBSPACE
};
#endif


/*    POLYCON.CPP - Implementation of Polyconic projection

      Formulas from Snyder pag. 129-131

*/
#include <geo/polycon.h>
#include <mlib/convert.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

Polyconic::Polyconic (const Params& params) :
  Projection (params)
{
  init ();
}

Polyconic& Polyconic::operator= (const Params& p)
{
  par = p;
  init ();
  return *this;
}

void Polyconic::init ()
{
  m0 = ellipsoid().lm (ref_latitude());
  double e2 = ellipsoid ().e2 ();
  c[0] = ((-0.01953125*e2 - 0.046875)*e2 - 0.250)*e2 + 1;
  c[1] = ((0.0439453125*e2 + 0.093750)*e2 + 0.375)*e2 * 2;
  c[2] = (0.0439453125*e2 + 0.05859375)*e2*e2 * 4;
  c[3] = -0.01139322916667*e2*e2*e2 * 6;
}

errc Polyconic::geo_xy (double *x, double *y, double lat, double lon) const
{
  double l = lon - ref_longitude();
  if (lat == 0.)
  {
    *x = ellipsoid().a ()*l;
    *y = -m0;
  }
  else
  {
    double ee = l*sin (lat);
    double n = ellipsoid().rn (lat) / tan (lat);
    *x = n*sin (ee);
    *y = ellipsoid().lm (lat) - m0 + n*(1 - cos (ee));
  }
  *x += false_east();
  *y += false_north();

  return ERR_SUCCESS;
}

errc Polyconic::xy_geo (double x, double y, double *lat, double *lon) const
{
  x -= false_east();
  y -= false_north();

  double x_a = x / ellipsoid().a ();
  if (y == -m0)
  {
    *lat = 0.;
    *lon = x_a + ref_longitude();
  }
  else
  {
    double aa = (m0 + y) / ellipsoid().a ();
    double bb = x_a*x_a + aa*aa;
    double cc;
    double phin;
    double phin1 = aa;
    double slat;
    int iter = 0;
    do 
    {
      phin = phin1;
      cc = sqrt (1 - ellipsoid().e2 ()*(slat = sin (phin))*slat)*tan (phin);
      double ma = ellipsoid().lm (phin) / ellipsoid().a ();
      double mn1 = m1 (phin);
      phin1 = phin - (aa*(cc*ma + 1) - ma - 0.5*(ma*ma + bb)*cc) / (ellipsoid().e2 ()*sin (2 * phin)*(ma*ma + bb - 2 * aa*ma) / (4 * cc) + (aa - ma)*(cc*mn1 - 2 / sin (2 * phin)) - mn1);
    } while (fabs (phin1 - phin) > 0.1*MAS && iter++ < 10);

    if (iter >= 10)
      return errc (GEOERR_NCONV);

    *lat = phin1;
    *lon = asin (x*cc / ellipsoid().a ()) / sin (phin1) + ref_longitude ();
  }
  return ERR_SUCCESS;
}

double Polyconic::h (double lat, double lon) const
{
  double e2_ = ellipsoid().e2 ();
  double scale;
  double l = lon - ref_longitude();
  if (lat != 0.)
  {
    double slat = sin (lat);
    double clat = cos (lat);
    double ee = l*slat;
    double d = atan ((ee - sin (ee)) / (1 / (clat*clat) - cos (ee) - e2_*slat*slat / (1 - e2_*slat*slat)));
    scale = (1 - e2_ + 2 * (1 - e2_*slat*slat)*sin (ee / 2)*sin (ee / 2) / (tan (lat)*tan (lat))) / ((1 - e2_)*cos (d));
  }
  else
    scale = (m1 (lat) + l*l / 2.) / (1 - e2_);

  return scale;
}

double Polyconic::m1 (double lat) const
{
  return c[0] - c[1] * cos (2 * lat) + c[2] * cos (4 * lat) + c[3] * cos (6 * lat);
}

#ifdef MLIBSPACE
};
#endif

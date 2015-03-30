/*!
  \file ELLIP.CPP - Ellipsoid class implementation

*/

#include <geo/ellip.h>
#include <assert.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/*!
  \class Ellipsoid
  Define an ellipsoid using the semi major axis and flattening.
  All other significant values are calculated based on a and f.
  Note that "f" is defined as (a-b)/a
*/


/// Known ellipsoids
static struct ell_data
{
  Ellipsoid::well_known wk;
  char *name;
  double a;
  double f_1;
} ellip[] = {
  { Ellipsoid::WGS_84,                "WGS-84",                   6378137.,     298.257223563 },
  { Ellipsoid::AIRY,                  "Airy",                     6377563.396,  299.3249646 },
  { Ellipsoid::ATS_77,                "ATS-77",                   6378135.0,    298.257 },
  { Ellipsoid::AUSTRALIAN,            "Australian National",      6378160.,     298.25 },
  { Ellipsoid::AIRY_MODIFIED,         "Airy Modified",            6377340.189,  299.3249646 },
  { Ellipsoid::BESSEL_1841,           "Bessel 1841",              6377397.155,  299.1528128 },
  { Ellipsoid::CLARKE_1866,           "Clarke 1866",              6378206.4,    294.9786982 },
  { Ellipsoid::CLARKE_1880,           "Clarke 1880",              6378249.145,  293.465 },
  { Ellipsoid::CLARKE_1880_ARC,       "Clarke-1880 (Arc)",        6378249.145,  293.4663077 },
  { Ellipsoid::CLARKE_1880_IGN,       "Clarke 1880 (IGN)",        6378249.200,  293.466021294, },
  { Ellipsoid::EVEREST,               "Everest",                  6377276.345,  300.8017 },
  { Ellipsoid::EVEREST_MALAY_SING,    "Everest (Malay. & Sing)",  6377304.063,  300.8017 },
  { Ellipsoid::EVEREST_INDIA_1956,    "Everest (India 1956)",     6377301.243,  300.8017 },
  { Ellipsoid::EVEREST_SABAH_SARWAK,  "Everest (Sabah Sarawak)",  6377298.556,  300.8017 },
  { Ellipsoid::EVEREST_MALAYSIA_1969, "Everest (Malaysia 1969)",  6377295.664,  300.8017 },
  { Ellipsoid::EVEREST_PAKISTAN,      "Everest (Pakistan)",       6377309.613,  300.8017 },
  { Ellipsoid::FISCHER_1960,          "Fischer 1960 (Mercury)",   6378166.,     298.3 },
  { Ellipsoid::FISCHER_1960_MODIFIED, "Fischer 1960 Modified",    6378155.,     298.3 },
  { Ellipsoid::FISCHER_1968,          "Fischer 1968",             6378150.,     298.3 },
  { Ellipsoid::GRS_1967,              "GRS-1967",                 6378160.,     298.247167427 },
  { Ellipsoid::GRS_1980,              "GRS-1980",                 6378137.,     298.257222101 },
  { Ellipsoid::HELMERT_1906,          "Helmert 1906",             6378200.,     298.3 },
  { Ellipsoid::HOUGH,                 "Hough",                    6378270.,     297. },
  { Ellipsoid::INDONESIAN_1974,       "Indonesian 1974",          6378160.,     298.247 },
  { Ellipsoid::INTERNATIONAL,         "International",            6378388.,     297. },
  { Ellipsoid::KRASSOVSKY,            "Krassovsky",               6378245.,     298.3 },
  { Ellipsoid::SOUTH_AMERICAN_1969,   "SA-1969",                  6378160.,     298.25 },
  { Ellipsoid::WGS_60,                "WGS-60",                   6378165.,     298.3 },
  { Ellipsoid::WGS_66,                "WGS-66",                   6378145.,     298.25 },
  { Ellipsoid::WGS_72,                "WGS-72",                   6378135.,     298.26 },
};

/// Create an ellipsoid object with the given a and f values.
Ellipsoid::Ellipsoid (double a, double f) :
a_ (a),
f_ (f),
c (0)
{
  e2_ = 2.*f_ - f_*f_;
  e_ = sqrt (e2_);
  int idx;
  if ((idx = find_wkidx ()) >= 0)
    name_ = ellip[idx].name;
  else
    name_ = 0;
}

/// Create a well-known ellipsoid
Ellipsoid::Ellipsoid (well_known wk) :
a_ (6378137.),
f_ (1./298.257223563), //just to have some reasonable defaults if wk is out of bounds
c (0)
{
  name_ = known (wk, &a_, &f_);
  e2_ = 2.*f_ - f_*f_;
  e_ = sqrt (e2_);
}

/// Copy Constructor
Ellipsoid::Ellipsoid (const Ellipsoid& ell) :
a_ (ell.a_),
f_ (ell.f_),
c (0),
e2_ (ell.e2_),
e_ (ell.e_),
name_ (ell.name_)
{
}

/// Assignment operator
Ellipsoid& Ellipsoid::operator= (const Ellipsoid& rhs)
{
  delete c;
  c = 0;
  a_ = rhs.a_;
  f_ = rhs.f_;
  e2_ = rhs.e2_;
  e_ = rhs.e_;
  name_ = rhs.name_;
  return *this;
}

///  Return radius of curvature in the prime vertical
///  \f$ rn = \frac a{\sqrt{1-e^2*\sin(lat)^2}} \f$
double Ellipsoid::rn (double lat) const
{
  double slat;
  return a_/sqrt( 1-e2_*(slat=sin(lat))*slat);
}

/// Return radius of curvature in the prime meridian
/// \f$ rm = \frac{a*(1-e^2)}{\sqrt{[1-e^2*\sin(lat)^2)]^3}} \f$
double Ellipsoid::rm (double lat) const
{
  double slat, root;
  return a_*(1-e2_)/((root=sqrt(1-e2_*(slat=sin(lat))*slat))*root*root);
}

/// Return length from Equator along meridian
double Ellipsoid::lm (double lat) const
{
  if (!c)
  {
    const_cast<double*>(c) = new double[4];
    c[0] = ((-0.01953125*e2_ - 0.046875)*e2_ -0.250)*e2_ +1;
    c[1] = ((0.0439453125*e2_ + 0.093750)*e2_ + 0.375)*e2_;
    c[2] = (0.0439453125*e2_ + 0.05859375)*e2_*e2_;
    c[3] = -0.01139322916667*e2_*e2_*e2_;
  }
  return a_*(c[0]*lat - c[1]*sin(2*lat) + c[2]*sin(4*lat) + c[3]*sin(6*lat));
}

/*!
  Return and and ellipsoidal parameters a and f of a well-known ellipsoid

  \param wk       well-known ellipsoid index
  \param pa       pointer to semi-major axis
  \param pf       pointer to flattening
  \return         ellipsoid name or NULL if index out of bounds

  The function can be used to enumerate the table of well-known ellipsoids by calling it
  with increasing index values until it returns NULL.
*/
const char *Ellipsoid::known (well_known wk, double *pa, double *pf)
{
  const int max_index = sizeof(ellip)/sizeof(ell_data)-1;
  int i;
  for (i = 0; i < max_index; i++)
  {
    if (ellip[i].wk == wk)
      break;
  }
  if (i < max_index)
  {
    if (pa)
      *pa = ellip[i].a;
    if (pf)
      *pf = 1./ellip[i].f_1;
    return ellip[i].name;
  }
  else
    return NULL;
}

/*!
  Search the table of known ellipsoids for a matching ellipsoid and return it's index.
  If not found, the function returns a negative value.
*/
int Ellipsoid::find_wkidx ()
{
  const int max_index = sizeof (ellip) / sizeof (ell_data) - 1;
  for (int i = 0; i < max_index; i++)
  {
    if ( fabs(a_ - ellip[i].a) < 1e-4 && fabs( f_ - 1./ellip[i].f_1) <1e-10 )
      return i;
  }
  return -1;
}

/*!
  Conversion from geographical coordinates to geocentric.

  Formulas from "GPS Satellite Surveying" by Alfred Leick page 184
*/
void Ellipsoid::geo_ECEF (double lat, double lon, double height, double *x, double *y, double *z) const
{
  double slat = sin(lat);
  double clat = cos(lat);
  double r = rn( lat );
  *x = (r + height)*clat*cos(lon);
  *y = (r + height)*clat*sin(lon);
  *z = ((1-e2_)*r + height)*slat;
}

/*!
  Conversion from geocentric to geographical coordinates.
  
  Formulas from "GPS Satellite Surveying" by Alfred Leick page 184-185.
  If both x and y are 0 longitude is set to 0.
*/
void Ellipsoid::ECEF_geo (double *lat, double *lon, double *height, double x, double y, double z) const
{
  double rho = hypot( x, y );
  if ( rho == 0. )
  {  //deal with North and South Pole cases
    *lat = (z>0)? M_PI/2 : -M_PI/2;
    *lon = 0.;
    *height = fabs(z)-b();
  }
  else
  {
    double lat0, lat1;
    double rn;
    lat0 = atan( 1/(1-e2_) * z/rho );
    lat1 = lat0+1;
    while (fabs(lat0-lat1) > 9e-10)  /*approx 0.1 mm*/
    {
      double slat = sin( lat0 );
      rn = a_/sqrt(1.-e2_*slat*slat);
      lat1 = lat0;
      lat0 = atan( (z + e2_*rn*slat)/rho );
    }
    *lon = atan2( y, x );
    *lat = lat0;
    *height = rho/cos(lat0) - rn;
  }
}

/*!
  Calculate geodetic (great circle) distance between 2 points using Gauss mid-latitude solution. 
  
  Formulas from "GPS Satellite Surveying" by Alfred Leick page 281-282
*/
double Ellipsoid::gcirc (double lat1, double lon1, double lat2, double lon2, double *az)
{
  double mid_phi = (lat1 + lat2)/2.;
  double dphi = lat2-lat1;
  double dlam = lon2-lon1;
  double smp = sin( mid_phi );
  double cmp = cos( mid_phi );

  double eta2 = ep2()*cmp*cmp;
  double V4 = (1-eta2)*(1-eta2);
  double t2 = tan( mid_phi )*tan(mid_phi);

  double C[8] = { rm(mid_phi), rn(mid_phi), 1./24., (1+9.*eta2*eta2*t2)/(24.*V4 ),
      (1-2*eta2)/24., eta2*(1-t2)/(8*V4), (1+eta2)/12., (3+8*eta2)/(24*V4) };

  double ssin = C[1]*dlam*cmp*(1-C[2]*dlam*dlam*smp*smp+C[3]*dphi*dphi);
  double scos = C[0]*cos(dlam/2)*dphi*(1+C[4]*dlam*dlam*cmp*cmp+C[5]*dphi*dphi);
  double dalf = dlam*smp*(1+C[6]*dlam*dlam*cmp*cmp+C[7]*dphi*dphi);
  if (az)
    *az = atan2( ssin, scos ) - dalf/2;
  return hypot (ssin, scos);
}

/*!
  Calculate rhumb line distance between 2 points.
*/
double Ellipsoid::rhumb (double lat1, double lon1, double lat2, double lon2, double *az)
{
  double dist;

  double sl1 = sin( lat1 );
  double sl2 = sin( lat2 );

  double azm = atan2( lon2-lon1,
            log( (1+sl2)/(1-sl2)*(1-sl1)/(1+sl1) *
            pow( (1-e_*sl2)/(1+e_*sl2)*(1+e_*sl1)/(1-e_*sl1), e_ ))
            );

  if ( az )
    *az = azm;

  if ( lat1 == lat2 )
    dist = fabs (rn(lat1)* cos(lat1) * (lon1-lon2));
  else
    dist = fabs ((lm(lat2)-lm(lat1))/cos(azm));
  return dist;
}

#ifdef MLIBSPACE
};
#endif

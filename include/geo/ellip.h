#pragma once

/*!
  \file ELLIP.H -  Ellipsoid class definition

*/
#include <mlib/defs.h>
#include <mlib/mathval.h>

#include <math.h>


#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

class Ellipsoid
{
public:
  Ellipsoid(double a, double f);
  ~Ellipsoid ();
  double a() const;
  double f() const;
  double b() const;
  double e() const;
  double e2() const;
  double ep() const;
  double ep2() const;
  double t (double lat) const;
  double q (double lat) const;
  double m (double lat) const;
  double beta (double lat) const;
  double rn( double lat ) const;
  double rm( double lat ) const;
  double lm( double lat ) const;
  double GCirc( double lat1, double lon1, double lat2, double lon2, double *az = 0 );
  double RLine( double lat1, double lon1, double lat2, double lon2, double *az = 0 );
  const char *Name() const;
  static const char *Known( int i, double *a=NULL, double *f=NULL );
  void Geo2ECEF( double lat, double lon, double height, double *x, double *y, double *z ) const;
  void ECEF2Geo( double *lat, double *lon, double *height, double x, double y, double z ) const;
  int operator ==( const Ellipsoid& other ) const;
  int operator !=( const Ellipsoid& other ) const;
  static const Ellipsoid WGS84;

private:
  double a_, f_, e2_, e_;
  double *c;
  static const char *FindName( double a, double f );
};

///integer exponentiation function
template <typename T>
T ipow (T base, int exp)
{
  T result = 1;
  while (exp)
  {
    if (exp & 1)
      result *= base;
    exp >>= 1;
    base *= base;
  }
  return result;
};

/*==================== INLINE FUNCTIONS ===========================*/

/// Create an ellipsoid object with the given a and f values.
inline
Ellipsoid::Ellipsoid(double a, double f):
a_(a),
f_(f),
c(0)
{
	e2_ = 2.*f_ - f_*f_;
	e_ = sqrt( e2_ );
}

///Destructor
inline
Ellipsoid::~Ellipsoid()
{
  delete c;
}

/// Return semi-major axis
inline
double Ellipsoid::a() const
{
	return a_;
}

/// Return flattening \f$ f = \frac{a-b}{a} \f$
inline
double Ellipsoid::f() const
{
	return f_;
}

/// Return semiminor axis \f$ b = a*( 1-f ) \f$
inline
double Ellipsoid::b() const
{
	return a_ * (1. - f_);
}

/// Return squared value of first eccentricity \f$ e2 = e^2 \f$
inline
double Ellipsoid::e2() const
{
	return e2_;
}

/// Return first eccentricity \f$ e = \sqrt{\frac{a^2 - b^2}{a^2}} \f$
inline
double Ellipsoid::e() const
{
	return e_;
}

/// Return squared value of second eccentricity \f$ ep2 = e'^2 \f$
inline
double Ellipsoid::ep2() const
{
	return e2_/(1-e2_);
}

/// Return second eccentricity \f$ e' = \sqrt{ \frac{a^2 - b^2}{b^2}} \f$
inline
double Ellipsoid::ep() const
{
	return e_/sqrt(1-e2_);
}

/*!
  Auxiliary function:
  \f$ t = \frac{1}{2*e}*\ln{\frac{1-e*\sin \phi}{1+e*\sin \phi}} \f$
*/
inline
double Ellipsoid::t(double phi) const
{
  double sphi = sin(phi);
  return log((1-e_*sphi)/(1+e_*sphi))/(2*e_);
}

/*!
  Auxiliary function:
  \f$ q = (1-e^2)*(\frac{\sin \phi}{1-e^2*\sin^2 \phi} - t(\phi)) \f$

  (Synder formula 3-12)
*/
inline 
double Ellipsoid::q (double phi) const
{
  double sphi=sin(phi);
  return (1-e2_)*(sphi/(1-e2_*sphi*sphi)-t(phi));
}

/*!
  Authalic latitude:
  \f$ \beta = \arcsin{\frac{q(\phi)}{q(\pi/2)}} \f$

  (Snyder formula 3-11)
*/
inline
double Ellipsoid::beta (double phi) const
{
  return asin(q(phi)/q(M_PI_2));
}

/*!
  Auxiliary function:
  \f$ m = \frac{\cos \phi}{\sqrt{1-e^2*\sin^2 \phi}} \f$

  (Synder formula 14-15)
*/
inline
double Ellipsoid::m(double phi) const
{ 
  double sphi=sin(phi); 
  return sqrt((1-sphi*sphi)/(1-e2_*sphi*sphi));
}

/// Return ellipsoid's name or NULL if not known
inline 
const char *Ellipsoid::Name() const
{
	return FindName( a_, f_ );
};

/// Return TRUE if the two ellipsoids are equal (same a and f values)
inline 
int Ellipsoid::operator ==( const Ellipsoid& other ) const
{
	return (a_ == other.a_) && (f_ == other.f_);
}

/// Return TRUE if the two ellipsoids are different
inline
int Ellipsoid::operator !=( const Ellipsoid& other ) const
{
	return !(*this == other);
}

#ifdef MLIBSPACE
};
#endif

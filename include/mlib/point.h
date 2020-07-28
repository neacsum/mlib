/*!
  \file point.h Definition of Point template class

  (c) Mircea Neacsu 2017

*/
#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

#define _USE_MATH_DEFINES
#include <math.h>

#include <ostream>

namespace mlib {


/*!
  \class Point
  \ingroup geom

  Template class which builds a 2D point from a pair of
  coordinates. It has functions to compute distance between two points,
  azimuth from north, as well as basic operators (comparison, I/O).
*/

template <typename T> struct point_traits {
  static T tolerance () { return 1e-7; };
};

template <> struct point_traits<int> {
  static int tolerance () {return 0;};
};

/// Generic 2D point
template<typename T> class Point
{
public:
  typedef point_traits<T> traits;
  T x, y;

  Point (T a, T b);
  Point ();

  double azimuth (const Point<T>& P2) const;
  double angle (const Point<T>& P1, const Point<T>& P2) const;

  Point<T> operator + (const Point<T>& p) const   { return { x + p.x, y + p.y }; }
  Point<T> operator - (const Point<T>& p) const   { return{ x - p.x, y - p.y }; }
  Point<T> operator * (double scalar) const       { return{ x*scalar, y*scalar }; }
  Point<T> operator / (double scalar) const       { return{ x / scalar, y / scalar }; }

  /// unary minus operator
  Point<T> operator - () const                    { return { -x, -y }; }

  ///dot product operator
  T operator * (const Point<T>& p) const          { return x*p.x + y*p.y; }
  
  Point<T>& operator += (const Point<T>& other)   { x += other.x; y += other.y; return *this; }
  Point<T>& operator -= (const Point<T>& other)   { x -= other.x; y -= other.y; return *this; }
  Point<T>& operator *= (double scalar)           { x *= scalar;  y *= scalar;  return *this; }
  Point<T>& operator /= (double scalar)           { x /= scalar;  y /= scalar;  return *this; }

  double distance (const Point<T>& P2) const;
  double magnitude () const                       { return hypot (x, y); }
  int operator ==(const Point<T>& p) const;
  int operator !=(const Point<T>& p) const;

  bool leftof (const Point<T>& a, const Point<T>& b) const;
  bool collinear (const Point<T>& a, const Point<T>& b) const;
  void rotate (double angle);
};

/// Scalar multiplication function makes scalar multiplication commutative
template <class T>
Point<T> operator *(double scalar, const Point<T>& p)
{
  return{ p.x*scalar, p.y*scalar };
}

/// Alias for magnitude function
template <class T>
double abs (const Point<T>& p)
{
  return p.magnitude ();
}


template <class T>
std::ostream& operator << (std::ostream& s, const Point<T>& p)
{
  s << '(' << p.x << ',' << p.y << ')';
  return s;
}


/// Specialization of Point using double as underlining type
typedef Point<double> dpoint;

//Member functions templates --------------------------------------------------

/// Build a Point from a pair of T's
template<class T>
Point<T>::Point( T a, T b ) :
  x(a),y(b)
{
}

/// Default constructor
template<class T>
Point<T>::Point() :
  x(0),y(0)
{
}

/*!
  Return azimuth from North of line this-P2

  0<= azimuth < 2*M_PI
*/
template<class T>
double Point<T>::azimuth (const Point<T>& P2) const
{
  if ( P2 == *this )
    return 0.;
  else
  {
    double t=atan2(P2.x-x, P2.y-y );
    return (t>0)? t : 2*M_PI+t ;
  }
}

/// Return TRUE if p and this are closer than tolerance
template<class T>
int Point<T>::operator == (const Point<T>& p) const
{
  return distance (p) < point_traits<T>::tolerance ();
}

/// Return TRUE if p and this apart by more than tolerance
template<class T>
int Point<T>::operator != (const Point<T>& p) const
{
  return !(p == *this);
}

/// Return distance between this and P2
template<class T>
double Point<T>::distance (const Point<T>& P2) const
{
  return hypot (x-P2.x, y-P2.y);
}

/*!
  Return inside angle P1-this-P2.

  0 <= angle < M_PI
  degenerated angles (p1 == this or p2 == this) are 0
*/
template<class T>
double Point<T>::angle (const Point<T>& P1, const Point<T>& P2) const
{
  if (P1 == *this || P2 == *this)
    return 0.;

  double cang = ((P1.x - x)*(P2.x - x) + (P1.y - y)*(P2.y - y)) / (distance (P1)*distance (P2));
  return (cang <= -1) ? M_PI : (cang >= 1) ? 0. : 
    acos (cang);
}

/// Return true if this is left of the line (a,b)
template<class T>
bool Point<T>::leftof (const Point<T>& a, const Point<T>& b) const
{
  return (a.x - x)*(b.y - y) - (a.y - y)*(b.x - x) > point_traits<T>::tolerance ();
}

/// Return true if points a, this, b are collinear
template <class T>
bool Point<T>::collinear (const Point& a, const Point& b) const
{
  return ::abs((a.x - x)*(b.y - y) - (a.y - y)*(b.x - x)) <= point_traits<T>::tolerance ();
}

template <class T>
void Point<T>::rotate (double angle)
{
  double c, s;
  double x1 = x*(c = cos (angle)) - y*(s = sin (angle));
  double y1 = x*s + y*c;
  x = x1; y = y1;
}

}

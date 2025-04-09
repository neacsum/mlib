/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file point.h Definition of mlib::Point template class

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <mlib/convert.h>

#include <ostream>

namespace mlib {

/*!
  \class point
  \ingroup geom

  Template class which builds a 2D point from a pair of
  coordinates. It has functions to compute distance between two points,
  azimuth from north, as well as basic operators (comparison, I/O).
*/

/// Provides default tolerance value for Point class
template <typename T>
struct point_traits
{
  static T tolerance ()
  {
    return 1e-7;
  };
};

/// Provide tolerance for integer points
template <>
struct point_traits<int>
{
  static int tolerance ()
  {
    return 0;
  };
};

/// Generic 2D point
template <typename T>
class Point
{
public:
  typedef point_traits<T> traits;
  T x, y;

  Point (T a, T b);
  Point ();

  /// Return azimuth from North of line this-P2
  double azimuth (const Point<T>& P2) const;

  /// Return inside angle P1-this-P2.
  double angle (const Point<T>& P1, const Point<T>& P2) const;

  Point<T> operator+ (const Point<T>& p) const;
  Point<T> operator- (const Point<T>& p) const;
  Point<T> operator* (double scalar) const;
  Point<T> operator/ (double scalar) const;

  /// unary minus operator
  Point<T> operator- () const
  {
    return {-x, -y};
  }

  /// dot product operator
  T operator* (const Point<T>& p) const
  {
    return x * p.x + y * p.y;
  }

  Point<T>& operator+= (const Point<T>& other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }
  Point<T>& operator-= (const Point<T>& other)
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }
  Point<T>& operator*= (double scalar)
  {
    x *= scalar;
    y *= scalar;
    return *this;
  }
  Point<T>& operator/= (double scalar)
  {
    x /= scalar;
    y /= scalar;
    return *this;
  }

  double distance (const Point<T>& P2) const;
  double magnitude () const
  {
    return hypot (x, y);
  }
  int operator== (const Point<T>& p) const;
  int operator!= (const Point<T>& p) const;

  bool leftof (const Point<T>& a, const Point<T>& b) const;
  bool collinear (const Point<T>& a, const Point<T>& b) const;
  void rotate (double angle);
};

/// Scalar multiplication function makes scalar multiplication commutative
template <class T>
Point<T> operator* (double scalar, const Point<T>& p)
{
  return {p.x * scalar, p.y * scalar};
}

/// Alias for magnitude function
template <class T>
double abs (const Point<T>& p)
{
  return p.magnitude ();
}

/// Serializes a point to an output stream as a comma-separated pair of values
/// in brackets
template <class T>
std::ostream& operator<< (std::ostream& s, const Point<T>& p)
{
  s << '(' << p.x << ',' << p.y << ')';
  return s;
}

/// Specialization of Point using double as underlining type
typedef Point<double> dpoint;

// Member functions templates --------------------------------------------------

/// Build a Point from a pair of values
template <typename T>
Point<T>::Point (T a, T b)
  : x (a)
  , y (b)
{}

/// Default constructor
template <class T>
Point<T>::Point ()
  : x (0)
  , y (0)
{}

/// Vector addition
template <typename T>
Point<T> Point<T>::operator+ (const Point<T>& p) const
{
  return {x + p.x, y + p.y};
}

/// Vector subtraction
template <typename T>
Point<T> Point<T>::operator- (const Point<T>& p) const
{
  return {x - p.x, y - p.y};
}

/// Scalar multiplication
template <typename T>
Point<T> Point<T>::operator* (double scalar) const
{
  return {x * scalar, y * scalar};
}

/// Scalar division
template <typename T>
Point<T> Point<T>::operator/ (double scalar) const
{
  return {x / scalar, y / scalar};
}


/*!
  Azimuth is measured in a clockwise direction
  0<= azimuth < 2*M_PI
*/
template <typename T>
double Point<T>::azimuth (const Point<T>& P2) const
{
  if (P2 == *this)
    return 0.;
  else
  {
    double t = atan2 (P2.x - x, P2.y - y);
    return (t > 0) ? t : 2 * M_PI + t;
  }
}

/// Return TRUE if p and this are closer than tolerance
template <typename T>
int Point<T>::operator== (const Point<T>& p) const
{
  return distance (p) < point_traits<T>::tolerance ();
}

/// Return TRUE if p and this apart by more than tolerance
template <typename T>
int Point<T>::operator!= (const Point<T>& p) const
{
  return !(p == *this);
}

/// Return distance between this and P2
template <typename T>
double Point<T>::distance (const Point<T>& P2) const
{
  return hypot (x - P2.x, y - P2.y);
}

/*!
  0 <= angle < M_PI
  degenerated angles (p1 == this or p2 == this) are 0
*/
template <typename T>
double Point<T>::angle (const Point<T>& P1, const Point<T>& P2) const
{
  if (P1 == *this || P2 == *this)
    return 0.;

  double cang =
    ((P1.x - x) * (P2.x - x) + (P1.y - y) * (P2.y - y)) / (distance (P1) * distance (P2));
  return (cang <= -1) ? M_PI : (cang >= 1) ? 0. : acos (cang);
}

/// Return `true` if this is left of the line (a,b)
template <typename T>
bool Point<T>::leftof (const Point<T>& a, const Point<T>& b) const
{
  return (a.x - x) * (b.y - y) - (a.y - y) * (b.x - x) > point_traits<T>::tolerance ();
}

/// Return `true` if points a, this, b are collinear
template <typename T>
bool Point<T>::collinear (const Point& a, const Point& b) const
{
  return ::abs ((a.x - x) * (b.y - y) - (a.y - y) * (b.x - x)) <= point_traits<T>::tolerance ();
}

template <typename T>
void Point<T>::rotate (double angle)
{
  auto [s, c] = sincos (angle);
  double x1 = x * c - y * s;
  double y1 = x * s + y * c;
  x = x1;
  y = y1;
}


} // namespace mlib

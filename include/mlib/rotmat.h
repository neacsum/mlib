#pragma once
/*!
  \file rotmat.h Definition of a rotation calculator class

  (c) Mircea Neacsu 2017. All rights reserved.
*/

#if __has_include("defs.h")
#include "defs.h"
#endif

namespace mlib {

///  3D Rotation Calculator
class RotMat
{
public:
  /// Build an identity rotation matrix
  RotMat ();

  /// Build rotation matrix in order Z, Y, X (yaw, pitch, roll)
  RotMat (double rx, double ry, double rz);

  /// Rotation by X (roll) axis
  void x_rotation (double angle);

  /// Rotation by Y (pitch) axis
  void y_rotation (double angle);

  /// Rotation by Z (yaw) axis
  void z_rotation (double angle);

  /// Rotate a 3D point.
  void rotate (double& x, double& y, double& z) const;

  /// Rotate a vector containing the x, y, z coordinates
  void rotate (double* vec) const;

  /// Return reference to rotation matrix (3x3)
  double (&matrix ())[3][3];

private:
  double r[3][3];
  void multiply (double m[3][3]);
};

inline double (&RotMat::matrix ())[3][3]
{
  return r;
}

} // namespace mlib

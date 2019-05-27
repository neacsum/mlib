#pragma once
/*!
  \file rotmat.h Definition of a rotation calculator class

  (c) Mircea Neacsu 2017. All rights reserved.
*/

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

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
  void rotate (double *vec) const;

private:
  double r[3][3];
  void multiply (double m[3][3]);
};

#ifdef MLIBSPACE
}
#endif

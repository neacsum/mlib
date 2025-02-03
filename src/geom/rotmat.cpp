/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file rotmat.cpp Implementation of a rotation calculator class

#include <mlib/mlib.h>
#pragma hdrstop
#include <math.h>

namespace mlib {

RotMat::RotMat ()
  : r{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}
{}

/*!
  Create a rotation matrix with specified rotation angles.
  \param rx rotation around X-axis (radians)
  \param ry rotation around Y-axis (radians)
  \param rz rotation around Z-axis (radians)

  Rotations are applied in the order z, y, x (Tait-Bryant convention).
*/
RotMat::RotMat (double rx, double ry, double rz)
  : RotMat ()
{
  z_rotation (rz);
  y_rotation (ry);
  x_rotation (rx);
}

void RotMat::x_rotation (double angle)
{
  double rx[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  rx[1][1] = rx[2][2] = cos (angle);
  rx[1][2] = -(rx[2][1] = sin (angle));
  multiply (rx);
}

void RotMat::y_rotation (double angle)
{
  double ry[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  ry[0][0] = ry[2][2] = cos (angle);
  ry[2][0] = -(ry[0][2] = sin (angle));
  multiply (ry);
}

void RotMat::z_rotation (double angle)
{
  double rz[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  rz[0][0] = rz[1][1] = cos (angle);
  rz[0][1] = -(rz[1][0] = sin (angle));
  multiply (rz);
}

void RotMat::rotate (double* vec) const
{
  double t[3];
  for (int i = 0; i < 3; i++)
  {
    t[i] = 0;
    for (int j = 0; j < 3; j++)
      t[i] += vec[j] * r[i][j];
  }
  for (int i = 0; i < 3; i++)
    vec[i] = t[i];
}

void RotMat::rotate (double& x, double& y, double& z) const
{
  double t[3];
  t[0] = x;
  t[1] = y;
  t[2] = z;
  rotate (t);
  x = t[0];
  y = t[1];
  z = t[2];
}

void RotMat::multiply (double m[3][3])
{
  double t[3][3];

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
    {
      t[i][j] = 0;
      for (int k = 0; k < 3; k++)
        t[i][j] += r[i][k] * m[k][j];
    }

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      r[i][j] = t[i][j];
}

} // namespace mlib

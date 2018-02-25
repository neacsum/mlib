#include <utpp/utpp.h>
#include <mlib/point.h>

using namespace MLIBSPACE;

SUITE (Point)
{

TEST (point_dist) {
  dpoint O, A (3, 0), B (0, 4);
  double d = A.distance (B);
  CHECK_CLOSE (5, d, dpoint::traits::tolerance());
}

TEST (point_angle) {
  dpoint O, A (1, 0), B (0, 2), C(1,1);
  double ang = O.angle(A, B);
  CHECK_CLOSE (M_PI / 2, ang, 1e-7);

  ang = O.angle (A, C);
  CHECK_CLOSE (M_PI / 4, ang, 1e-7);

  ang = O.angle (B, C);
  CHECK_CLOSE (M_PI / 4, ang, 1e-7);
}

TEST (point_leftof) {
  dpoint O, A (3, 0), B (0, 4);
  CHECK (O.leftof (A, B));
}

TEST (point_collinear) {
  dpoint O, A (1, 1), B (2, 2);
  CHECK (O.collinear (A, B));
}

}


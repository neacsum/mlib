#include <utpp/utpp.h>
#include <mlib/point.h>

using namespace mlib;


SUITE (Point)
{

// Most tests use arbitrary values. Don't look for any hidden meaning.

TEST (distance) {
  dpoint O, A (3, 0), B (0, 4);
  double d = A.distance (B);
  CHECK_CLOSE (5, d, dpoint::traits::tolerance());
}

TEST (angle) {
  dpoint O, A (1, 0), B (0, 2), C(1,1);
  double ang = O.angle(A, B);
  CHECK_CLOSE (M_PI / 2, ang, 1e-7);

  ang = O.angle (A, C);
  CHECK_CLOSE (M_PI / 4, ang, 1e-7);

  ang = O.angle (B, C);
  CHECK_CLOSE (M_PI / 4, ang, 1e-7);
}

TEST (leftof) {
  dpoint O, A (3, 0), B (0, 4);
  CHECK (O.leftof (A, B));

  dpoint A1{ 2, 16 }, B1{ 1, 9 }, C1{ 1, 10 };
  CHECK (!C1.leftof (A1, B1));
}

TEST (collinear) {
  dpoint O, A (1, 1), B (2, 2);
  CHECK (O.collinear (A, B));

  dpoint A1{ 2, 16 }, B1{ 1, 9 }, C1{ -1, -5 };
  CHECK (C1.collinear (A1, B1));
}

TEST (add) {
  dpoint A{ -10, 12 }, B{ 5, -10 };

  dpoint C = A + B;
  CHECK_EQUAL (dpoint(-5, 2), C);
  A += B;
  CHECK_EQUAL (C, A);
}

TEST (subtract) {
  dpoint A{ 6, 5 }, B{ 5, 4 };

  dpoint C = B - A;
  CHECK_EQUAL (dpoint (-1, -1), C);
}

TEST (scalar_multiplication)
{
  dpoint A{ 6, 5 };

  dpoint B = A * 3;
  CHECK_EQUAL (dpoint (18, 15), B);

  dpoint C = 3 * A;
  CHECK_EQUAL (B, C);

  dpoint D = B / 3;
  CHECK_EQUAL (A, D);
}

TEST (distance_magnitude)
{
  dpoint A{ 6, 5 }, B{ 5, 4 };
  dpoint C = B - A;

  CHECK (C.magnitude () == A.distance (B));
}

TEST (rotate)
{
  dpoint A{ 6, 5 };
  dpoint B = -A;
  A.rotate (M_PI);
  CHECK_EQUAL (B, A);
}
}


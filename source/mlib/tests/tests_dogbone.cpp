#include <mlib/dogbone.h>
#include <utpp/utpp.h>
#include <mlib/mathval.h>

float dims[4] = { 0.73f, 0.89f, 0.96f, 1.2f };

TEST (dogbone_ok)
{
  float bucket;
  CHECK_EQUAL (0, dogbone (dims, (float)(15 * M_PI / 180.), (float)(120 * M_PI / 180), &bucket));
  CHECK_CLOSE (135.8 * M_PI/180, bucket, 0.01);
}

TEST (dogbone_inverse)
{
  float dog;
  CHECK_EQUAL (0, invbone (dims, (float)(15 * M_PI / 180.), &dog, (float)(135.8 * M_PI / 180)));
  CHECK_CLOSE (120 * M_PI / 180, dog, 0.01);

}
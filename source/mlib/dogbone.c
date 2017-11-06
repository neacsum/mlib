#include <mlib/dogbone.h>
#include <math.h>
#include <mlib/mathval.h>

#define ab_ dims[0]
#define bc_ dims[1]
#define cd_ dims[2]
#define ad_ dims[3]

/*!
  Calculate bucket angle for an excavator bucket where inclinometer is installled on dogbone.
  The dogbone quadrangle has the following sides:
    AB - on stick
    BC - on bucket
    CD - link
    AD - dogbone

  \param dims dimensions of dogbone quadrangle in the order AB, BC, CD, AD
  \param stick stick angle from horizontal (in radians)
  \param dog inclinometer angle from horizontal (in radians)
  \param bucket pointer to computed bucket angle

  \return 0 success
  \return -1 impossible geometry

*/
int dogbone (float *dims, float stick, float dog, float *bucket)
{
  float bad, bd, c_abd, c_dbc;
  bad = (float)(M_PI - dog + stick);  //angle BAD
  if (fabs (bad) > M_PI)
    return -1;
  bd = (float)sqrt (ab_*ab_ + ad_*ad_ - 2 * ab_*ad_*cos (bad));  //diagonal of quadrangle
  c_abd = (bd*bd + ab_*ab_ - ad_*ad_) / (2 * ab_*bd);   //cosine of ABD angle
  if (fabs (c_abd) >= 1)
    return -1;
  c_dbc = (bd*bd + bc_*bc_ - cd_*cd_) / (2 * bd*bc_);  //cosine of DBC angle
  if (fabs (c_dbc) >= 1)
    return -1;
  *bucket = (float)(acos (c_abd) + acos (c_dbc) + stick);
  return 0;
}

/*!
  Calculate dogbone angle for an excavator bucket when bucket angle is known.

  \param dims dimensions of dogbone quadrangle in the order AB, BC, CD, AD
  \param stick stick angle from horizontal (in radians)
  \param dog pointer to computed dogbone angle from horizontal (in radians)
  \param bucket angle (in radians)

  \return 0 success
  \return -1 impossible geometry

*/
int invbone (float *dims, float stick, float *dog, float bucket)
{
  float abc, ac, c_bac, c_cad;
  abc = bucket - stick;
  ac = (float)sqrt (ab_*ab_ + bc_*bc_ - 2 * ab_*bc_*cos (abc));
  c_bac = (ab_*ab_ + ac*ac - bc_*bc_) / (2 * ab_*ac);
  if (fabs (c_bac) >= 1)
    return -1;
  c_cad = (ad_*ad_ + ac*ac - cd_*cd_) / (2 * ad_*ac);
  if (fabs (c_cad) >= 1)
    return -1;
  *dog = (float)(M_PI - (acos (c_bac) + acos (c_cad) - stick));
  return 0;
}

/*!
  \file BORDER.CPP Implementation of Border object

  (c) Mircea Neacsu 2017
*/

#include <mlib/border.h>
#include <stdio.h>
#include <algorithm>
#include <utf8/utf8.h>

using namespace std;

/*!
  \defgroup geom Geometry 
  \brief Geometry concepts and algorithms
*/

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/*!
  \class Border 
  \ingroup geom
  \brief Representation of a simple, non-intersecting polygon that
  partitions the 2D space in two regions.

  The polygon is represented by its vertexes and it is always assumed that
  there is a segment joining the last point with the first point. A Border object
  can be stored in a text file where each line represents a vertex. The last
  vertex defines what is considered the "inside" of the polygon: if the point
  lays inside the polygon, it is an "island" border. If the last point is
  outside the polygon, it is a "hole" border.
*/

/*!
  Create an empty border object
*/
Border::Border ()
{
  closing.x = 0;
  closing.y = 0;
  closing_outside = 0;
}

/*!
  Load a border object from a text file.
*/
Border::Border (const char *fname)
{
  dpoint p;

  closing.x = 0;
  closing.y = 0;
  closing_outside = 0;
  FILE *f = utf8::fopen (fname, "r");
  if (!f)
    return;

  while (!feof (f))
  {
    char ln[256];
    fgets (ln, sizeof (ln), f);
    sscanf (ln, "%lf%lf", &p.x, &p.y);
    vertex.push_back (p);
  }

  fclose (f);
  closing = vertex.back ();
  vertex.pop_back ();
  closing_outside = !inside (closing.x, closing.y);
}

void Border::add (double x, double y)
{
  dpoint p;
  p.x = x;
  p.y = y;
  vertex.push_back (p);
}

void Border::close (double x, double y)
{
  closing.x = x;
  closing.y = y;
  closing_outside = !inside (closing.x, closing.y);
}

/*!
  Check if a point is inside the border.

  \param x - X coordinate of the point
  \param y - Y coordinate of the point
  \return true if point is inside the border

  Algorithm adapted from W. Randolph Franklin <wrf@ecse.rpi.edu>
  http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html

*/
bool Border::inside (double x, double y)
{
  int c = 0;

  deque<dpoint>::iterator pi = vertex.begin ();
  deque<dpoint>::iterator pj = vertex.end ();

  if (pi == pj)
    return 0;   //empty border

  pj--;

  while (pi != vertex.end ())
  {
    if (((pi->y > y) != (pj->y > y)) &&
      (x < (pj->x - pi->x) * (y - pi->y) / (pj->y - pi->y) + pi->x))
      c = !c;
    pj = pi++;
  }
  return (c != closing_outside);
}

#ifdef MLIBSPACE
}
#endif

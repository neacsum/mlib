#include <utpp/utpp.h>

#include <geo/filegeo.h>
#include <geo/convert.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

struct geo_file {
  geo_file ();
  ~geo_file ();
  union {
    geo_header hdr;
    float vals[sizeof(geo_header)/sizeof(float) +1];
  } rec;
};

geo_file::geo_file ()
{
  const int N = sizeof (geo_header) / sizeof (float) + 1;
  memset (&rec, 0, sizeof (rec));
  strcpy (rec.hdr.ext.rident, "Test GEO File");
  rec.hdr.ext.a = A_WGS84;
  rec.hdr.ext.f = F1_WGS84;
  memcpy (rec.hdr.pgm, "COASTALO", 8);
  rec.hdr.x01 = 10;
  rec.hdr.y01 = 15;
  rec.hdr.dx1 = 1;
  rec.hdr.dy1 = 1;
  rec.hdr.nc = N;
  rec.hdr.nr = N;
  rec.hdr.nz = 1;
  FILE *f = fopen ("test.geo", "wb");
  fwrite (&rec, sizeof (rec), 1, f);
  memset (&rec, 0, sizeof (rec));
  for (int i = 1; i < N; i++)
  {
    for (int j = 1; j < N; j++)
    {
      rec.vals[j] = (float)(i*N+j);
    }
    fwrite (&rec, sizeof (rec), 1, f);
  }
  fclose (f);
}

geo_file::~geo_file ()
{
  _unlink ("test.geo");
}

TEST_FIXTURE (geo_file, Interp_basic)
{
  FileGEO g ("test.geo");
  double xmin, xmax, ymin, ymax;
  g.limits (xmin, xmax, ymin, ymax);

  CHECK_EQUAL (10., xmin/D2R);
  CHECK_EQUAL (15., ymin/D2R);
}
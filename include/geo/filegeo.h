#pragma once
/*!
  \file FILEGEO.H -  FileGEO class definition
*/
#include <geo/grid.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

#pragma pack (push, 1)
  /// Header for .GEO and .LLS files
  struct geo_header
  {
    union {                                                                   //0
      char ident[56];   ///< data description
      struct {
        char rident[30];///< reduced description
        double a;       ///< source ellipsoid
        double f;
      } ext;
    };
    char pgm[8];        ///< creator program                                   56
    long nc;            ///< number of columns (lon values)                    64
    long nr;            ///< number of rows (lat values)                       68
    long nz;            ///< number of values in a cell (1 for GEO 2 or 3 for LLS) 72
    float x01;          ///< min lon value (in degrees)                        76
    float dx1;          ///< lon spacing (in degrees)                          80
    float y01;          ///< min lat value (in degrees)                        84
    float dy1;          ///< lat spacing (in degrees)                          88
    float angle1;       ///< unused (must be 0)                                92
  };                                                                           //96
#pragma pack (pop)

/*
  An extension of the CORPSCON and GEOID96 format allows storing of the ellipsoidal
  values (a and f) for which the interpolation applies.
*/
class FileGEO : public Gridded
{
public:
  FileGEO (const char *filename, bool west_positive = false);
  ~FileGEO ();
  const char* description () const;
  erc Interpolate (double lat, double lon, double *interp);

private:
  float *rec1, *rec2;
  int recl;
  int ilat, ilon;
  FILE *file;
  geo_header header;

};

#ifdef MLIBSPACE
};
#endif

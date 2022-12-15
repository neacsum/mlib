/*!
  \file convert.cpp Conversion functions

  (c) Mircea Neacsu 2017
*/
#include <mlib/convert.h>
#include <mlib/ipow.h>
#include <utf8/utf8.h>

constexpr double epsilon = 1e-10;

namespace mlib {

double inline pow10 (int x)
{
  return ipow (10., x);
}

//safe replacement for sprintf to a string
inline
size_t strprintf (std::string& str, const char* fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  auto sz = vsnprintf (nullptr, 0, fmt, args);
  str.resize (sz+1); //leave space for terminating null
  sz = vsnprintf (const_cast<char*>(str.c_str ()), sz+1, fmt, args);
  str.resize (sz);
  return sz;
}

double deg_reduce (double value)
{
  double val = fmod (value, 360);
  if (val < 0)
    val += 360;
  return val;
}

/*!
  Conversion from degrees to ASCII string. Format is controlled by flags
  settings and precision.
*/
std::string degtoa (double degrees, int flags, int precision)
{
  std::string str;
  char  sign;
  int   deg_width, width;

  width = precision + 3;
  if (flags & LL_LAT)
  {
    sign = (char)((degrees >= 0.) ? 'N' : 'S');
    deg_width = 2;
  }
  else
  {
    sign = (char)((degrees >= 0.) ? 'E' : 'W');
    deg_width = 3;
  }

  degrees = fabs (degrees);
  if (flags & LL_SEC)
  {
    double  dd, mm;
    degrees = modf (modf (degrees + .5 * pow10 (-precision - 1) / 3600., &dd) * 60., &mm) * 60.;
    strprintf (str, u8"%0*.0lf°%02.0lf'%0*.*lf\"%c", deg_width, dd, mm, width, precision,
      degrees, sign);
  }
  else if (flags & LL_MIN)
  {
    double  dd;
    degrees = modf (degrees + .5 * pow10 (-precision - 1) / 60., &dd) * 60.;
    strprintf (str, u8"%0*.lf°%0*.*lf\'%c", deg_width, dd, width, precision, degrees, sign);
  }
  else
  {
    width = deg_width + precision + 1;
    strprintf (str, u8"%0*.*lf°%c", width, precision, degrees + .5 * pow10 (-precision - 1),
      sign);
  }

  return str;
}

/*!
  Conversion from string to decimal degrees.
    Some valid strings are:
    - 130°45'25.34566W
    - 130D45'25.34566E
    - 65D45M25.34567S
    - 130.56789W
    - 85°30.45678N
*/
double atodeg (const std::string& str)
{
  int     mm;
  double  val, ss;
  char* ptrm;

  if (str.empty ())
    return 0;

  const char* cstr = str.c_str ();

  int dd = (int)strtol (cstr, &ptrm, 10);
  int sign = (dd < 0) ? -1 : 1;
  dd = abs (dd);

  if (toupper (*ptrm) == 'D' || utf8::rune (ptrm) == U'°')
  {
    utf8::next (ptrm);
    char* ptrs;
    mm = (int)strtol (ptrm, &ptrs, 10);
    if (*ptrs == '\'' || toupper (*ptrs) == 'M')
    {
      ptrs++;
      ss = atof (ptrs);
      val = dd + mm / 60. + ss / 3600.;
    }
    else if (*ptrs == '.')
    {
      ss = atof (ptrm);
      val = dd + ss / 60.;
    }
    else
      val = dd + mm / 60.;
  }
  else if (*ptrm == '.')
    val = abs(stof (str));
  else
    val = dd;
  if (val != 0. && (str.find('S') != std::string::npos
                 || str.find ('W') != std::string::npos))
    val = -val;

  return val*sign;
}

} //end namespace

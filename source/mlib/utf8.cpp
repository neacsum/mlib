/*!
  \file UTF8.CPP - UTF8 Conversion functions
  (c) Mircea Neacsu 2014-2017

  \defgroup utf8 UTF-8 
  \brief UTF-8 related functions

  Functions in this group facilitate handling of I18N problems using
  the strategy advocated by [UTF-8 Everywhere](http://utf8everywhere.org/)
*/
#include <windows.h>

#include <mlib/utf8.h>
#include <vector>

using namespace std;

///@{
/*!
  Conversion from wide character to UTF-8
  \ingroup utf8

  \param  s input string
  \return UTF-8 character string
*/
string narrow(const wchar_t *s)
{
  int wsz = (int)wcslen(s);
  int nsz = WideCharToMultiByte (CP_UTF8, 0, s, wsz, 0, 0, 0, 0);
  string out (nsz, 0);
  if (nsz)
    WideCharToMultiByte (CP_UTF8, 0, s, wsz, &out[0], nsz, 0, 0);
  return out;
}

string narrow (const wstring &s)
{
  int wsz = (int)s.size ();
  int nsz = WideCharToMultiByte (CP_UTF8, 0, &s[0], wsz, 0, 0, 0, 0);
  string out (nsz, 0);
  if (nsz)
    WideCharToMultiByte (CP_UTF8, 0, &s[0], wsz, &out[0], nsz, 0, 0);
  return out;
}
///@}

///@{
/*!
  Conversion from UTF-8 to wide character
  \ingroup utf8

  \param  s input string
  \return wide character string
*/
wstring widen (const char *s)
{
  int nsz = (int)strlen (s);
  int wsz = MultiByteToWideChar (CP_UTF8, 0, s, nsz, 0, 0);
  wstring out (wsz, 0);
  if (wsz)
    MultiByteToWideChar (CP_UTF8, 0, s, nsz, &out[0], wsz);
  return out;
}


wstring widen(const string &s)
{
  int nsz = (int)s.size ();
  int wsz = MultiByteToWideChar (CP_UTF8, 0, &s[0], nsz, 0, 0);
  wstring out (wsz, 0);
  if (wsz)
    MultiByteToWideChar (CP_UTF8, 0, &s[0], nsz, &out[0], wsz);
  return out;
}
///@}

/*!
  Converts wide byte command arguments to an array of pointers 
  to UTF-8 strings.

  \ingroup utf8

  \param  argc Pointer to an integer that contains number of parameters
  \return array of pointers to each command line parameter

  The space allocated for strings and array of pointers should be freed
  by calling free_utf8argv()
*/
char **get_utf8argv (int *argc)
{
  wchar_t **wargv = CommandLineToArgvW ( GetCommandLineW(), argc);
  char** uargv = (char **)malloc (*argc*sizeof(char*));
  for (int i=0; i<*argc; i++)
  {
    int nc = WideCharToMultiByte (CP_UTF8, 0, wargv[i], -1, 0, 0, 0, 0);
    uargv[i] = (char *)malloc (nc+1);
    WideCharToMultiByte (CP_UTF8, 0, wargv[i], -1, uargv[i], nc, 0, 0);    
  }
  LocalFree (wargv);
  return uargv;
}

/*!
  Frees the memory allocated by get_utf8argv()
  \ingroup utf8

  \param  argc  number of arguments
  \param  argv  array of pointers to arguments
*/
void free_utf8argv (int argc, char **argv)
{
  for (int i = 0; i<argc; i++)
    free (argv[i]);
  free (argv);
}

/*!
  Converts wide byte command arguments to UTF-8 to a vector of UTF-8 strings.
  \ingroup utf8

  \return vector of UTF-8 strings
*/
vector<string> get_utf8argv ()
{
  int argc;
  vector<string> uargv;

  wchar_t **wargv = CommandLineToArgvW (GetCommandLineW(), &argc);
  for (int i=0; i<argc; i++)
    uargv.push_back (narrow (wargv[i]));
  LocalFree (wargv);
  return uargv;
}


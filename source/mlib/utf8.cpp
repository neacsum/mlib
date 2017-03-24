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
namespace utf8 {

///@{
/*!
  Conversion from wide character to UTF-8
  \ingroup utf8

  \param  s input string
  \return UTF-8 character string
  */
string narrow (const wchar_t* s)
{
  int wsz = (int)wcslen (s);
  int nsz = WideCharToMultiByte (CP_UTF8, 0, s, wsz, 0, 0, 0, 0);
  string out (nsz, 0);
  if (nsz)
    WideCharToMultiByte (CP_UTF8, 0, s, wsz, &out[0], nsz, 0, 0);
  return out;
}

string narrow (const wstring& s)
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
wstring widen (const char* s)
{
  int nsz = (int)strlen (s);
  int wsz = MultiByteToWideChar (CP_UTF8, 0, s, nsz, 0, 0);
  wstring out (wsz, 0);
  if (wsz)
    MultiByteToWideChar (CP_UTF8, 0, s, nsz, &out[0], wsz);
  return out;
}


wstring widen (const string& s)
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
  wchar_t **wargv = CommandLineToArgvW (GetCommandLineW (), argc);
  char** uargv = (char **)malloc (*argc*sizeof (char*));
  for (int i = 0; i < *argc; i++)
  {
    int nc = WideCharToMultiByte (CP_UTF8, 0, wargv[i], -1, 0, 0, 0, 0);
    uargv[i] = (char *)malloc (nc + 1);
    WideCharToMultiByte (CP_UTF8, 0, wargv[i], -1, uargv[i], nc, 0, 0);
  }
  LocalFree (wargv);
  return uargv;
}

/*!
  Frees the memory allocated by get_argv()
  \ingroup utf8

  \param  argc  number of arguments
  \param  argv  array of pointers to arguments
  */
void free_argv (int argc, char **argv)
{
  for (int i = 0; i < argc; i++)
    free (argv[i]);
  free (argv);
}

/*!
  Converts wide byte command arguments to UTF-8 to a vector of UTF-8 strings.
  \ingroup utf8

  \return vector of UTF-8 strings
  */
vector<string> get_argv ()
{
  int argc;
  vector<string> uargv;

  wchar_t **wargv = CommandLineToArgvW (GetCommandLineW (), &argc);
  for (int i = 0; i < argc; i++)
    uargv.push_back (narrow (wargv[i]));
  LocalFree (wargv);
  return uargv;
}

int mkdir (const char* dirname)
{
  return _wmkdir (widen (dirname).c_str ());
}

int mkdir (const string& dirname)
{
  return _wmkdir (widen (dirname).c_str ());
}

int rmdir (const char* dirname)
{
  return _wrmdir (widen (dirname).c_str ());
}

int rmdir (const string& dirname)
{
  return _wrmdir (widen (dirname).c_str ());
}

int chdir (const char* dirname)
{
  return _wchdir (widen (dirname).c_str ());
}

int chdir (const string& dirname)
{
  return _wchdir (widen (dirname).c_str ());
}

std::string getcwd ()
{
  wchar_t tmp[_MAX_PATH];
  if (_wgetcwd (tmp, _countof (tmp)))
    return narrow (tmp);
  else
    return string ();
}

int access (const char *path, int mode)
{
  return _waccess (widen (path).c_str (), mode);
}

int access (const std::string& path, int mode)
{
  return _waccess (widen (path).c_str (), mode);
}

int remove (const char* dirname)
{
  return _wremove (widen (dirname).c_str ());
}

int remove (const string& dirname)
{
  return _wremove (widen (dirname).c_str ());
}

int rename (const char* oldname, const char* newname)
{
  wstring oldn = widen (oldname);
  wstring newn = widen (newname);

  return _wrename (oldn.c_str (), newn.c_str ());
}

int rename (const string& oldname, const string& newname)
{
  wstring oldn = widen (oldname);
  wstring newn = widen (newname);

  return _wrename (oldn.c_str (), newn.c_str ());
}

void splitpath (const char* path, char* drive, char* dir, char* fname, char* ext)
{
  wstring wpath = widen (path);
  wchar_t wdrive[_MAX_DRIVE];
  wchar_t wdir[_MAX_DIR];
  wchar_t wfname[_MAX_FNAME];
  wchar_t wext[_MAX_EXT];

  _wsplitpath (wpath.c_str (),
               drive ? wdrive : 0,
               dir ? wdir : 0,
               fname ? wfname : 0,
               ext ? wext : 0);
  if (drive)
    strcpy (drive, narrow (wdrive).c_str ());
  if (dir)
    strcpy (dir, narrow (wdir).c_str ());
  if (fname)
    strcpy (fname, narrow (wfname).c_str ());
  if (ext)
    strcpy (ext, narrow (wext).c_str ());
}

void splitpath (const char* path, string& drive, string& dir, string& fname, string& ext)
{
  wstring wpath = widen (path);
  wchar_t wdrive[_MAX_DRIVE];
  wchar_t wdir[_MAX_DIR];
  wchar_t wfname[_MAX_FNAME];
  wchar_t wext[_MAX_EXT];

  _wsplitpath (wpath.c_str (), wdrive, wdir, wfname, wext);
  drive = narrow (wdrive);
  dir = narrow (wdir);
  fname = narrow (wfname);
  ext = narrow (wext);
}

} //end namespace

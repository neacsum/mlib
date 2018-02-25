/*!
  \file UTF8.CPP - UTF8 Conversion functions
  (c) Mircea Neacsu 2014-2017

  \defgroup utf8 UTF-8 
  \brief UTF-8 related functions

  Functions in this group facilitate handling of I18N problems using
  the strategy advocated by [UTF-8 Everywhere](http://utf8everywhere.org/)
*/
#include <windows.h>
#include <sys/stat.h>
#include <mlib/utf8.h>
#include <vector>
#include <assert.h>

using namespace std;
namespace utf8 {

/*!
  Conversion from wide character to UTF-8
  \ingroup utf8

  \param  s input string
  \return UTF-8 character string
*/
std::string narrow (const wchar_t* s)
{
  if (!s)
    return string ();
  int wsz = (int)wcslen (s);
  int nsz = WideCharToMultiByte (CP_UTF8, 0, s, wsz, 0, 0, 0, 0);
  string out (nsz, 0);
  if (nsz)
    WideCharToMultiByte (CP_UTF8, 0, s, wsz, &out[0], nsz, 0, 0);
  return out;
}

/*!
  Conversion from wide character to UTF-8
  \ingroup utf8

  \param  s input string
  \return UTF-8 character string
*/
std::string narrow (const std::wstring& s)
{
  int wsz = (int)s.size ();
  int nsz = WideCharToMultiByte (CP_UTF8, 0, &s[0], wsz, 0, 0, 0, 0);
  string out (nsz, 0);
  if (nsz)
    WideCharToMultiByte (CP_UTF8, 0, &s[0], wsz, &out[0], nsz, 0, 0);
  return out;
}

///\{
/*!
  Conversion from UTF-8 to wide character
  \ingroup utf8

  \param  s input string
  \return wide character string
  */
std::wstring widen (const char* s)
{
  if (!s)
    return wstring ();
  int nsz = (int)strlen (s);
  int wsz = MultiByteToWideChar (CP_UTF8, 0, s, nsz, 0, 0);
  wstring out (wsz, 0);
  if (wsz)
    MultiByteToWideChar (CP_UTF8, 0, s, nsz, &out[0], wsz);
  return out;
}


std::wstring widen (const std::string& s)
{
  int nsz = (int)s.size ();
  int wsz = MultiByteToWideChar (CP_UTF8, 0, &s[0], nsz, 0, 0);
  wstring out (wsz, 0);
  if (wsz)
    MultiByteToWideChar (CP_UTF8, 0, &s[0], nsz, &out[0], wsz);
  return out;
}
///\}

/*!
  Conversion from UTF-8 to UTF-32
  \ingroup utf8
  
  \param s UTF-8 encoded string
  \return UTF-32 encoded string
*/
std::u32string runes (const std::string& s)
{
  u32string str;
  for (auto p = s.begin (); p != s.end (); next (s, p))
    str.push_back (rune (p));
  return str;
}


/*!
  Conversion from UTF32 to UTF8
  \param s UTF-32 encoded string
  \return UTF-8 encoded string

  Each character in the input string should be a valid UTF-32 code point
  ( <0x10FFFF)
*/
std::string narrow (const std::u32string& s)
{
  string str;
  for (auto p = s.begin (); p != s.end (); p++)
  { 
    assert (*p < 0x10ffff);

    if (*p < 0x7f)
      str.push_back ((char)*p);
    else if (*p < 0x7ff)
    {
      str.push_back ((char)(0xC0 | *p >> 6));
      str.push_back (0x80 | *p & 0x3f);
    }
    else if (*p < 0xFFFF)
    {
      str.push_back ((char)(0xE0 | *p >> 12));
      str.push_back (0x80 | *p >> 6 & 0x3f);
      str.push_back (0x80 | *p & 0x3f);
    }
    else
    {
      str.push_back ((char)(0xF0 | *p >> 18));
      str.push_back (0x80 | *p >> 12 & 0x3f);
      str.push_back (0x80 | *p >> 6 & 0x3f);
      str.push_back (0x80 | *p & 0x3f);
    }
  }
  return str;
}


/*!
  Counts number of characters in an UTF8 encoded string
  
  \ingroup utf8
  \param s UTF8-encoded string

  \note Algorithm from http://canonical.org/~kragen/strlen-utf8.html
*/
size_t length (const std::string& s)
{
  size_t nc = 0;
  auto p = s.begin ();
  while (p != s.end ())
  {
    if ((*p++ & 0xC0) != 0x80)
      nc++;
  }
  return nc;
}

/// Return current Unicode code point.
char32_t rune (std::string::const_iterator p)
{
  int rune = 0;
  if ((*p & 0x80) == 0)
  {
    rune = *p;
  }
  else if ((*p & 0xE0) == 0xc0)
  {
    rune = (*p++ & 0x1f) << 6;
    assert ((*p & 0xC0) == 0x80);
    rune += *p & 0x3f;
  }
  else if ((*p & 0xf0) == 0xE0)
  {
    rune = (*p++ & 0x0f) << 12;
    assert ((*p & 0xC0) == 0x80);
    rune += (*p++ & 0x3f) << 6;
    assert ((*p & 0xC0) == 0x80);
    rune += (*p & 0x3f);
  }
  else
  {
    rune = (*p++ & 0x07) << 18;
    assert ((*p & 0xC0) == 0x80);
    rune += (*p++ & 0x3f) << 12;
    assert ((*p & 0xC0) == 0x80);
    rune += (*p++ & 0x3f) << 6;
    assert ((*p & 0xC0) == 0x80);
    rune += (*p & 0x3f);
  }
  return rune;
}

/// Verifies if string is a valid utf8 string
bool valid (const char *s)
{
  int rem = 0;
  while (*s)
  {
    if (rem)
    {
      if ((*s & 0xC0) != 0x80)
        return false;
      else
        rem--;
    }
    else if (*s & 0x80)
    {
      if ((*s & 0xC0) == 0x80)
        return false;

      rem = ((*s & 0xE0) == 0xC0) ? 1 :
        ((*s & 0xF0) == 0xE0) ? 2 : 3;
    }
    s++;
  }
  return !rem;
}

/// Advance iterator to next rune
bool next (const std::string& s, std::string::const_iterator& p)
{
  int rem = 0;
  if (p == s.end ())
    return true;    //don't advance past end

  do
  {
    if ((*p & 0xC0) == 0x80)
    {
      if (rem)
        rem--;
      else
        return false;   //missing continuation byte
    }
    else if ((*p & 0xE0) == 0xC0)
      rem = 1;
    else if ((*p & 0xF0) == 0xE0)
      rem = 2;
    else if ((*p & 0xF8) == 0xF0)
      rem = 3;
    p++;
  } while (rem && p != s.end ());

  return !rem; // rem == 0 if sequence is complete
}

/*!
  Converts wide byte command arguments to an array of pointers
  to UTF-8 strings.

  \ingroup utf8

  \param  argc Pointer to an integer that contains number of parameters
  \return array of pointers to each command line parameter

  The space allocated for strings and array of pointers should be freed
  by calling free_utf8argv()
*/
char** get_argv (int *argc)
{
  wchar_t **wargv = CommandLineToArgvW (GetCommandLineW (), argc);
  char** uargv = new char*[*argc];
  for (int i = 0; i < *argc; i++)
  {
    int nc = WideCharToMultiByte (CP_UTF8, 0, wargv[i], -1, 0, 0, 0, 0);
    uargv[i] = new char[nc + 1];
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
    delete argv[i];
  delete argv;
}

/*!
  Converts wide byte command arguments to UTF-8 to a vector of UTF-8 strings.
  \ingroup utf8

  \return vector of UTF-8 strings
*/
std::vector<std::string> get_argv ()
{
  int argc;
  vector<string> uargv;

  wchar_t **wargv = CommandLineToArgvW (GetCommandLineW (), &argc);
  for (int i = 0; i < argc; i++)
    uargv.push_back (narrow (wargv[i]));
  LocalFree (wargv);
  return uargv;
}
///\{
/*!
  Creates a new directory
  \ingroup utf8

  \param dirname UTF-8 path for new directory
  \return true if successful, false otherwise
*/
bool mkdir (const char* dirname)
{
  return (_wmkdir (widen (dirname).c_str ()) == 0);
}

bool mkdir (const std::string& dirname)
{
  return (_wmkdir (widen (dirname).c_str ()) == 0);
}
///\}


///\{
/*!
  Deletes a directory
  \ingroup utf8

  \param dirname UTF-8 path of directory to be removed
  \return true if successful, false otherwise
  */
bool rmdir (const char* dirname)
{
  return (_wrmdir (widen (dirname).c_str ()) == 0);
}

bool rmdir (const std::string& dirname)
{
  return (_wrmdir (widen (dirname).c_str ()) == 0);
}
///\}


///\{
/*!
  Changes the current working directory
  \ingroup utf8

  \param dirname UTF-8 path of new working directory
  \return true if successful, false otherwise
*/
bool chdir (const char* dirname)
{
  return (_wchdir (widen (dirname).c_str ()) == 0);
}

bool chdir (const std::string& dirname)
{
  return (_wchdir (widen (dirname).c_str ()) == 0);
}
///\}

///\{
/*!
  Changes the file access permissions
  \ingroup utf8

  \param filename UTF-8 name of file
  \param mode access permissions. Or'ed combination of:
              - _S_IWRITE write permission
              - _S_IREAD  read permission

  \return true if successful, false otherwise
*/

bool chmod (const char* filename, int mode)
{
  return (_wchmod (widen (filename).c_str (), mode) == 0);
}

bool chmod (const std::string& filename, int mode)
{
  return (_wchmod (widen (filename).c_str (), mode) == 0);
}
///\}

///Gets the current working directory as an UTF-8 encoded string
std::string getcwd ()
{
  wchar_t tmp[_MAX_PATH];
  if (_wgetcwd (tmp, _countof (tmp)))
    return narrow (tmp);
  else
    return string ();
}


///\{
/*!
  Determines if a file has the requested access permissions
  \ingroup utf8

  \param filename UTF-8 file path of new working directory
  \param mode required access:
              - 0 existence only
              - 2 write permission
              - 4 read permission
              - 6 read/write permission

  \return true if successful, false otherwise
*/
bool access (const char* filename, int mode)
{
  return (_waccess (widen (filename).c_str (), mode) == 0);
}

bool access (const std::string& filename, int mode)
{
  return (_waccess (widen (filename).c_str (), mode) == 0);
}
///\}


///\{
/*!
  Delete a file
  \ingroup utf8

  \param filename UTF-8 name of file to be deleted
  \return true if successful, false otherwise
*/
bool remove (const char* filename)
{
  return (_wremove (widen (filename).c_str ()) == 0);
}

bool remove (const std::string& filename)
{
  return (_wremove (widen (filename).c_str ()) == 0);
}
///\}

///\{
/*!
  Rename a file or directory
  \ingroup utf8

  \param oldname current UTF-8 encoded name of file or directory
  \param newname new UTF-8 name
  \return true if successful, false otherwise
*/
bool rename (const char* oldname, const char* newname)
{
  wstring oldn = widen (oldname);
  wstring newn = widen (newname);

  return (_wrename (oldn.c_str (), newn.c_str ()) == 0);
}

bool rename (const std::string& oldname, const std::string& newname)
{
  wstring oldn = widen (oldname);
  wstring newn = widen (newname);

  return (_wrename (oldn.c_str (), newn.c_str ()) == 0);
}
///\}

///\{
/*!
  Breaks a path name into components
  \ingroup utf8

  \param path   UTF-8 encoded full path
  \param drive  drive letter followed by colon (or NULL)
  \param dir    directory path (or NULL)
  \param fname  base filename (or NULL)
  \param ext    file extension including the leading period (.) (or NULL)
*/
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

void splitpath (const char* path, std::string& drive, std::string& dir, std::string& fname, std::string& ext)
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
///\}

} //end namespace

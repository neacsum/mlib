/*!
  \file BASENAME.CPP Implementation of Unix-like basename() and dirname()
  functions.

  (c) Mircea Neacsu 2017
*/
#include <mlib/basename.h>
#include <string>
#include <utf8/utf8.h>


using namespace std;

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/*! 
  \param filename pointer to filename with optional path
  \return     pointer to filename without path

  Forward slashes ( / ), backslashes ( \ ), or both may be used.

  If filename is a null pointer or points to an empty string, basename() returns 
  a pointer to the string ".".

  The function returns a pointer to a static storage that will be overwritten
  by a subsequent call to basename().

  The function can be used with UTF-8 encoded filenames.
*/

const char *basename (const char* filename)
{
  static string bname;
  
  //NULL or empty path gets treated as "."
  if (!filename || !*filename)
  {
    bname = ".";
    return bname.c_str();
  }

  string drive, dir, fname, ext;

  utf8::splitpath (filename, drive, dir, fname, ext);

  bname = fname + ext;
  
  if (bname.empty())
    bname = ".";

  return bname.c_str();
}

/*!
  \param filename pointer to filename
  \return         pointer to pathname

  Forward slashes ( / ), backslashes ( \ ), or both may be used.
  If the filename does not contain a '\\', then dirname() returns a pointer to
  the string ".". If filename is a null pointer or points to an empty string, 
  dirname() returns a pointer to the string "."

  The function returns a pointer to a static storage that will be overwritten
  by a subsequent call to dirname().

  The function can be used with UTF-8 encoded filenames.
*/
const char *dirname (const char *filename)
{
  static string dname;

  string drive, dir, fname, ext;
  //NULL or empty path gets treated as "."
  if (!filename || !*filename)
  {
    dname = ".";
    return dname.c_str();
  }

  utf8::splitpath (filename, drive, dir, fname, ext);
  dname = drive + dir;
  if (dname.empty())
    dname = ".";
  
  //remove terminating path separator
  if (dname.back() == '\\' || dname.back() == '/')
    dname.pop_back();

  return dname.c_str();
}

#ifdef MLIBSPACE
};
#endif

#include <windows.h>
#include <mlib/utf8.h>
/*!
  \file BASENAME.CPP - Implementation of Unix-like basename() and dirname()
  functions.
*/

using namespace std;

/*! 
  \param filename pointer to filename with optional path
  \return     pointer to filename without path

  Forward slashes ( / ), backslashes ( \ ), or both may be used.

  If filename is a null pointer or points to an empty string, basename() returns 
  a pointer to the string ".".

  The function returns a pointer to a static storage that will be overwritten
  by a subsequent call to basename().

  The function is not reentrant.
  The function can be used with UTF-8 encoded filenames.
*/

char *basename (const char* filename)
{
  wchar_t ext[_MAX_EXT], fname[_MAX_FNAME];
  static char bname[_MAX_FNAME + _MAX_EXT];

  //NULL or empty path gets treated as "."
  if (!filename || !*filename)
  {
    strcpy (bname, ".");
    return bname;
  }

  _wsplitpath (widen(filename).c_str(), NULL, NULL, fname, ext);
  strcpy (bname, narrow(fname).c_str());
  strcat (bname, narrow(ext).c_str());
  
  if (!strlen(bname))
    strcpy (bname, ".");

  return bname;
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

  The function is not reentrant.
  The function can be used with UTF-8 encoded filenames.
*/
char *dirname (const char *filename)
{
  static char dname[_MAX_PATH + _MAX_DRIVE];
  wchar_t drive[_MAX_DRIVE], path[_MAX_PATH];

  //NULL or empty path gets treated as "."
  if (!filename || !*filename)
  {
    strcpy (dname, ".");
    return dname;
  }

  _wsplitpath (widen(filename).c_str(), drive, path, NULL, NULL);
  strcpy (dname, narrow(drive).c_str());
  strcat (dname, narrow(path).c_str());
  if (!strlen(dname))
    strcpy (dname, ".");
  char *last = dname +strlen(dname)-1;
  
  //remove terminating path separator
  if (*last == '\\' || *last == '/')
    *last = 0;
  return dname;
}
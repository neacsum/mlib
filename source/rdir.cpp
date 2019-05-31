/*!
  \file RDIR.CPP Recursive directory functions

    (c) Mircea Neacsu 2019
*/
#ifndef UNICODE
#define UNICODE
#endif

#include <mlib/rdir.h>
#include <mlib/trace.h>
#include <assert.h>
#include <utf8/utf8.h>

using namespace std;

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/*!
  Recursively create a new directory
  \param dir path for new directory

  \return - 0 if new directory was created
          - ENOENT if path was not found
          - EEXIST if path already exists
*/
int r_mkdir (const char* dir)
{
  assert (dir && strlen(dir));

  string ldir;
  const char* p = dir;
  while (*p)
  {
    if (*p == '\\' || *p == '/')
    {
      if (!utf8::access (ldir, 0))
      {
        if (!utf8::mkdir (ldir))
          return ENOENT;
      }
    }
    ldir.push_back (*p++);
  }

  if (ldir.back () != '\\' && ldir.back () != '/')
  {
    //take care of last path segment
    if (utf8::access (ldir, 0))
      return EEXIST;
    else
    {
      if (!utf8::mkdir (ldir))
        return ENOENT;
    }
  }
  return 0;
}

/*!
  Recursively remove a directory
  \param dir path for directory to remove

  \return - 0 if new directory was removed
          - ENOTEMPTY if directory is not empty
*/
int r_rmdir (const char* dir)
{
  assert (dir && strlen(dir));

  string ldir(dir);
  if (ldir.back () == '\\' || ldir.back () == '/')
    ldir.pop_back ();

  while (1)
  {
    if (!utf8::rmdir (ldir))
      return ENOTEMPTY;

    //erase another path segment
    while (!ldir.empty () && ldir.back () != '\\' && ldir.back () != '/')
      ldir.pop_back ();

    if (ldir.empty ())
      return 0;

    ldir.pop_back ();

    if (ldir.back () == ':'
     || ldir == "."
     || ldir == "..")
      return 0;
  }
}

#ifdef MLIBSPACE
}
#endif

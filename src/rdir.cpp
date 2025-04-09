/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

#include <mlib/mlib.h>
#pragma hdrstop
#include <assert.h>
#include <utf8/utf8.h>

using namespace std;

namespace mlib {

/*!
  Recursively create a new directory
  \param dir path for new directory (UTF-8 encoded)

  \return - 0 if new directory was created
          - ENOENT if path was not found
          - EEXIST if path already exists
*/
int r_mkdir (const std::string& dir)
{
  assert (!dir.empty ());

  string ldir;
  auto p = dir.begin ();
  while (p != dir.end ())
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
    // take care of last path segment
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
  \param dir path for directory to remove (UTF-8 encoded)

  \return - 0 if new directory was removed
          - ENOTEMPTY if directory is not empty
*/
int r_rmdir (const std::string& dir)
{
  assert (!dir.empty ());

  string ldir (dir);
  if (ldir.back () == '\\' || ldir.back () == '/')
    ldir.pop_back ();

  while (1)
  {
    if (!utf8::rmdir (ldir))
      return ENOTEMPTY;

    // erase another path segment
    while (!ldir.empty () && ldir.back () != '\\' && ldir.back () != '/')
      ldir.pop_back ();

    if (ldir.empty ())
      return 0;

    ldir.pop_back ();

    if (ldir.back () == ':' || ldir == "." || ldir == "..")
      return 0;
  }
}

} // namespace mlib

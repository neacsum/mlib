/*!
  \file asset.cpp Implementation of asset class

  (c) Mircea Neacsu 2022. All rights reserved.
*/

#include <mlib/asset.h>
#include <mlib/rdir.h>
#include <mlib/trace.h>
#include <windows.h>

using namespace std;

namespace mlib {

/*!
  \class asset

  This class provides a mechanism for 'hiding' the different assets
  needed by a HTTP server (pages, CSS files, images, etc.) inside the
  EXE file.

  Each asset is stored as a user-defined resource of type RESFILE
  (defined as 256) and is identified by its ID. An asset can be written
  to a file. When the asset object goes out of scope the file is deleted.

  The resource file (.rc) should contain some lines like these:
    IDR_INDEX_HTML RESFILE  "index.html"
    IDR_MAIN_CSS   RESFILE  "main.css"
*/

/*!
   Load asset data
   \return pointer to asset data or 0 if an error occurs.
*/
const void* asset::data ()
{
  if (!loaded)
    load ();

  return ptr;
}

/*!
  Write the asset to a folder
  \param  path  root path for asset file (with or without terminating backslash)
  \return `true` if successful, `false` otherwise

  Asset filename is obtained by appending the asset name to the root path.
*/
bool asset::write (const std::string& path)
{
  //Load resource
  if (!loaded)
    load ();

  if (!loaded)
    return false; //load failed

  string tmp = path;
  int rc;

  //Root path must be terminated with '\' or '\\'
  if (tmp.back () != '/' && tmp.back () != '\\')
    tmp.push_back ('\\');

  //Name cannot start with '/' or '\\'
  int pstart = 0;
  if (name.front () == '/' || name.front () == '\\')
    pstart++;


  //Make sure all folders on path exist. If not we create them now
  size_t pend = name.find_last_of ("/\\");
  if (pend != string::npos)
    tmp += name.substr (pstart, pend);
  if ((rc = r_mkdir (tmp)) && rc != EEXIST)
    return false; //could not create path

  if (pend != string::npos)
    tmp += name.substr (pend);
  else
    tmp += name;
  if (keep && utf8::access (tmp, 0))
  {
    //permanent asset exists. Don't overwrite it (just pretend)
    written = true;
    return true;
  }
  FILE* f;
  f = utf8::fopen (tmp, "wb");
  if (!f)
    return false; // cannot open output file

  fullpath = tmp;
  TRACE ("Writing resource size %d file %s", sz, fullpath.c_str ());
  fwrite (ptr, sizeof (char), sz, f);
  fclose (f);

  written = true;
  return true;
}

/// Load a resource in memory
void asset::load ()
{
  ptr = nullptr;
  sz = 0;

  HMODULE handle = GetModuleHandle (NULL);
  HRSRC rc = FindResource (handle, MAKEINTRESOURCE (id),
    MAKEINTRESOURCE (RESFILE));
  if (!rc)
    return;

  HGLOBAL rcData = LoadResource (handle, rc);
  if (!rcData)
    return;

  sz = SizeofResource (handle, rc);
  ptr = LockResource (rcData);
  loaded = true;
}


}

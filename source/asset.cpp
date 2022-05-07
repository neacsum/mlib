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

static void* mem_resource (int name, int type, DWORD& size);

/*!
  \class asset

  This class provides a nice mechanism for 'hiding' the different
  assets needed by a HTTP server (pages, CSS files, images, etc.) inside the
  EXE file.

  Each asset is stored as a user-defined resource of type RESFILE
  (defined as 256) and is identified by its ID. This function writes the asset
  to a file.

  The resource file (.rc) should contain some lines like these:
    IDR_INDEX_HTML RESFILE  "index.html"
    IDR_MAIN_CSS   RESFILE  "main.css"

  \param  path  root path for all assets (with or without terminating backslash
  \return _true_ if successful
*/

/// write the asset to a folder
bool asset::write (const std::string& path)
{
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

  //Load resource
  DWORD size = 0;
  void* data = mem_resource (id, RESFILE, size);         //load resource...
  if (!data)
    return false;
  if (pend != string::npos)
    tmp += name.substr (pend);
  else
    tmp += name;
  FILE* f;
  f = utf8::fopen (tmp, "wb");        //... and write it
  if (!f)
    return false;
  fullpath = tmp;
  TRACE ("Writing resource size %d file %s", size, fullpath.c_str ());
  fwrite (data, sizeof (char), size, f);
  fclose (f);

  written = true;
  return true;
}

//Load a resource in memory
static void* mem_resource (int name, int type, DWORD& size)
{
  HMODULE handle = GetModuleHandle (NULL);
  HRSRC rc = FindResource (handle, MAKEINTRESOURCE (name),
    MAKEINTRESOURCE (type));
  if (!rc)
    return NULL;

  HGLOBAL rcData = LoadResource (handle, rc);
  if (!rcData)
    return NULL;
  size = SizeofResource (handle, rc);
  return LockResource (rcData);
}


}

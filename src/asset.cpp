/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

#include <mlib/mlib.h>
#pragma hdrstop
#include <utf8/utf8.h>

using namespace std;

namespace mlib {

/*!
  \class asset

  This class provides a mechanism for embedding different "assets"
  like those needed by a HTTP server (pages, CSS files, images, etc.),
  inside a Windows executable or DLL.

  Each asset is stored as a user-defined resource of type `RESFILE`
  (defined as 256) and is identified by its ID. An asset can be written
  to a file. When the asset object goes out of scope the file is deleted.

  ### Sample Usage ###

  The resource file (.rc):

  ```text
    IDR_INDEX_HTML RESFILE  "index.html"
    IDR_MAIN_CSS   RESFILE  "main.css"
  ```

  Program file:

  ```cpp
    mlib::asset page (IDR_INDEX_HTML, "web/index.html");
    mlib::asset css (IDR_MAIN_CSS, "web/css/main.css");
  //....
    page.write ("C:\\temp");
    css.write ("C:\\temp");
  ```
  This has created the file `c:\temp\web\index.html` and
  `c:\temp\web\css\main.css`. When the variables `page` and `css` go out of
  scope, the two files will be deleted (however, the directory structure remains
  in place).
*/


/*!
  Write the asset to a folder
  \param  path  root path for asset file
  \return `true` if successful, `false` otherwise

  Asset filename is obtained by appending the asset name to the root path.
  For persistent assets, if file already exists, it is not overwritten.
*/
bool asset::write (const std::filesystem::path& path)
{
  if (name.empty () || written || !load ())
    return false; // sanity checks

  fullpath = filesystem::absolute (path / name);
  error_code ec;
  filesystem::create_directories (fullpath.parent_path (), ec);
  if (ec)
    return false; // could not create path

  if (keep && filesystem::exists (fullpath))
  {
    // permanent asset exists. Don't overwrite it (just pretend)
    written = true;
    return true;
  }
  FILE* f;
  f = utf8::fopen (fullpath.generic_u8string (), "wb");
  if (!f)
    return false; // cannot open output file

  TRACE ("Writing resource size %d file %s", sz, fullpath.generic_u8string().c_str());
  fwrite (ptr, sizeof (char), sz, f);
  fclose (f);

  written = true;
  return true;
}

/// Load a resource in memory
bool asset::load ()
{
  if (loaded)
    return true; //already loaded

  ptr = nullptr;
  sz = 0;

  HMODULE handle = GetModuleHandle (NULL);
  HRSRC rc = FindResource (handle, MAKEINTRESOURCE (id), MAKEINTRESOURCE (RESFILE));
  if (!rc)
    return false;

  HGLOBAL rcData = LoadResource (handle, rc);
  if (!rcData)
    return false;

  sz = SizeofResource (handle, rc);
  ptr = LockResource (rcData);
  loaded = true;

  return true;
}

} // namespace mlib

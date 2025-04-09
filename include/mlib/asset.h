/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file asset.h Definition of mlib::asset class.

#pragma once

#ifndef RESFILE
/// Resource type for asset files
#define RESFILE 256
#endif

#ifndef RC_INVOKED //

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <string>
#include <filesystem>

namespace mlib {

/// Class for storing asset files in Windows resource data.
class asset
{
public:
  asset (int id_, const std::filesystem::path& name_ = std::filesystem::path (),
         bool persist = false);
  ~asset ();

  /// Load asset data and return a pointer to it.
  const void* data ();

  /// Return size of asset data or 0 if asset could not be loaded
  size_t size ();
  
  bool write (const std::filesystem::path& root_path);

  /// Delete asset file from disk.
  bool remove ();

  /// Relative asset name
  std::filesystem::path name;

private:
  bool load ();
  int id;
  bool written;
  bool loaded;
  bool keep; // do not delete asset file in destructor
  void* ptr;
  size_t sz;
  std::filesystem::path fullpath;
};

/*!
  Constructor for an asset object

  \param name_ asset file name
  \param id_ resource ID
  \param persist if `true` do not delete disk file when asset object is destructed
*/
inline
asset::asset (int id_, const std::filesystem::path& name_, bool persist)
  : name (name_)
  , id (id_)
  , written (false)
  , keep (persist)
  , loaded (false)
  , sz (0)
  , ptr (nullptr)
{}

/// Destructor. Delete asset file if it exists and is not persistent
inline
asset::~asset ()
{
  if (written && !keep)
    remove ();
}

/*!
  \note File is deleted even if it is a persistent asset.
  \return `true` if file was deleted
  \return `false` if file was not written or doesn't exist
*/
inline
bool asset::remove ()
{
  if (written)
  {
    written = false;
    return std::filesystem::remove (fullpath);
  }
  return false;
}

/// \return size of asset (in bytes)
inline
size_t asset::size ()
{
  if (!loaded)
    load ();
  return sz;
}

///  \return pointer to asset data or 0 if an error occurs.
inline
const void* asset::data ()
{
  if (!loaded)
    load ();

  return ptr;
}

} // namespace mlib
#endif //RC_INVOKED

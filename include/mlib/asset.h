/*!
  \file asset.h Definition of asset class.

  (c) Mircea Neacsu 2022
*/
#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <string>
#include <utf8/utf8.h>

#ifndef RESFILE
/// Resource type for asset files
#define RESFILE 256
#endif

namespace mlib {

/// Class for storing asset files in Windows resource data.
class asset
{
public:
  asset (int id_, const std::string& name_ = std::string (), bool persist = false);
  ~asset ();
  const void* data ();
  size_t size ();
  bool write (const std::string& path);
  bool remove ();

  const std::string name;

private:
  void load ();
  int id;
  bool written;
  bool loaded;
  bool keep; // do not delete asset file in destructor
  size_t sz;
  void* ptr;
  std::string fullpath;
};

/*!
  Constructor for an asset object

  \param name_ asset file name
  \param id_ resource ID
  \param persist if `true` do not delete disk file when asset object is destructed
*/
inline asset::asset (int id_, const std::string& name_, bool persist)
  : name (name_)
  , id (id_)
  , written (false)
  , keep (persist)
  , loaded (false)
  , sz (0)
  , ptr (nullptr)
{}

/// Destructor. Delete asset file if it exists
inline asset::~asset ()
{
  if (written && !keep)
    remove ();
}

/*
  Delete asset file from disk.

  File is deleted even if this is a persistent asset.
*/
inline bool asset::remove ()
{
  if (written)
  {
    written = false;
    return utf8::remove (fullpath);
  }
  return true;
}

/// Return size of asset (in bytes)
inline size_t asset::size ()
{
  return sz;
}
} // namespace mlib
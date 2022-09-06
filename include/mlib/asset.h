/*!
  \file asset.h Definition of asset class.

  (c) Mircea Neacsu 2022
*/
#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

#include <string>
#include <utf8/utf8.h>

#ifndef RESFILE
/// Resource type for asset files
#define RESFILE   256
#endif

namespace mlib {

/// Class for storing asset files in Windows resource data.
class asset
{
public:
  asset (const std::string& name_, int id_, bool keep=false);
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
*/
inline
asset::asset (const std::string& name_, int id_, bool keep_) 
  : name (name_), id (id_), written (false), keep(keep_), loaded(false), sz (0), ptr (nullptr)
{
}

/// Destructor. Delete asset file if it exists
inline
asset::~asset ()
{
  if (written && !keep)
    remove ();
}

/// Delete asset file from disk
inline
bool asset::remove ()
{
  if (written)
  {
    written = false;
    return utf8::remove (fullpath);
  }
  return true;
}

/// Return size of asset (in bytes)
inline
size_t asset::size ()
{
  return sz;
}
} //mlib namespace
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
#define RESFILE   256
#endif

namespace mlib {

/// Class for storing assets in Windows resource data.
class asset
{
public:
  asset (const std::string& name_, int id_) : name (name_), id (id_) , written (false) {};
  ~asset ();
  bool write (const std::string& path);
  bool remove ();

  const std::string name;
private:
  int id;
  bool written;
  std::string fullpath;
};

/// delete asset file from disk
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

/// Destructor. Deletes asset file if it exists
inline
asset::~asset ()
{
  if (written)
    remove ();
}


} //mlib namespace
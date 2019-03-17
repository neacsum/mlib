#pragma once
/*
  \file PROFILE.H Definition of Profile

*/

#if __has_include ("defs.h")
#include "defs.h"
#endif

#include <string>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

///Operations on INI files
class Profile
{
public:
  /// Constructor based on an existing or new INI file.
  Profile (const std::string& name);

  /// Default constructor uses a temporary file.
  Profile ();

  /// Copy constructor
  Profile (const Profile& p);

  /// Destructor
  ~Profile ();

  /// Assignment operator
  Profile& operator= (const Profile&);

  /// Return file name associated with this object
  const char *File () const
    { return filename; };

  /// Set the file name associated with this object
  void File (const std::string& filename);
  
  ///Get a string key
  size_t GetString (char *value, size_t len, const std::string& key, const std::string& section, const std::string& defval = std::string()) const;

  ///Return a string key
  std::string GetString (const std::string& key, const std::string& section, const std::string& defval = std::string ()) const;

  ///Return an integer key
  int GetInt (const std::string& key, const std::string& section, int defval=0) const;

  /// Return a floating point value
  double GetDouble (const std::string& key, const std::string& section, double defval=0.) const;

  ///Return a color specification key
  COLORREF GetColor (const std::string& key, const std::string& section, COLORREF defval=RGB(0,0,0)) const;

  ///Return a boolean key
  bool GetBool (const std::string& key, const std::string& section, bool defval=false) const;

  ///Return a font specification key
  HFONT GetFont (const std::string& key, const std::string& section, HFONT defval=NULL) const;

  ///Check for key existence
  bool HasKey (const std::string& key, const std::string& section) const;

  /// Write a string key
  bool PutString (const std::string& key, const std::string& value, const std::string& section);

  /// Write an integer key
  bool PutInt (const std::string& key, long value, const std::string& section);

  /// Write a font specification key
  bool PutFont (const std::string& key, HFONT font, const std::string& section);

  /// Write a boolean key
  bool PutBool (const std::string& key, bool value, const std::string& section);

  /// Write a color specification key
  bool PutColor (const std::string& key, COLORREF value, const std::string& section);

  /// Write a floating point value key
  bool PutDouble (const std::string& key, double value, const std::string& section, int dec = 2);

  /// Delete a key
  bool DeleteKey (const std::string& key, const std::string& section);

  /// Delete an entire section
  bool DeleteSection (const std::string& section);

  /// Copy all keys from one section to another.
  bool CopySection (const Profile& from_file, const char *from_sect, const char *to_sect=NULL);

  /// Return \b true if profile contains a non empty section with the given name
  bool HasSection (const std::string& section);

  /// Retrieve names of all keys in a section.
  int GetKeys (char *buffer, size_t sz, const std::string& section);

  /// Return the names of all sections in the INI file.
  int GetSections (char *sects, int sz);

private:

  char filename[MAX_PATH];
  bool temp_file;
};


#ifdef MLIBSPACE
}
#endif

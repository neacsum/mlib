#pragma once
/*
  \file PROFILE.H Definition of Profile

*/

#include "defs.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

#define INI_BUFFERSIZE  1024

///Operations on INI files
class Profile
{
public:
  /// Constructor based on an existing or new INI file.
  Profile (const char *name);

  /// Default constructor uses a temporary file.
  Profile ();

  /// Copy constructor
  Profile (const Profile& p);

  /// Destructor
  virtual ~Profile ();

  /// Assignment operator
  Profile& operator= (const Profile&);

  /// Return file name associated with this object
  const char *File () const
    { return filename; };

  /// Set the file name associated with this object
  void File (const char *filename);
  
  ///Get a string key
  virtual int GetString (char *value, int len, const char *key, const char *section, const char *defval="") const;

  ///Return an integer key
  int GetInt (const char *key, const char *section, int defval=0) const;

  /// Return a floating point value
  double GetDouble (const char *key, const char *section, double defval=0.) const;

  ///Return a color specification key
  COLORREF GetColor (const char *key, const char *section, COLORREF defval=RGB(0,0,0)) const;

  ///Return a boolean key
  bool GetBool (const char *key, const char *section, bool defval=false) const;

  ///Return a font specification key
  HFONT GetFont (const char *key, const char *section, HFONT defval=NULL) const;

  ///Check for key existence
  bool HasKey (const char *key, const char *section) const;

  /// Write a string key
  virtual bool PutString (const char *key, const char *value, const char *section);

  /// Write an integer key
  bool PutInt (const char *key, long value, const char *section);

  /// Write a font specification key
  bool PutFont (const char *key, HFONT font, const char *section);

  /// Write a boolean key
  bool PutBool (const char *key, bool value, const char *section);

  /// Write a color specification key
  bool PutColor (const char *key, COLORREF value, const char *section);

  /// Write a floating point value key
  bool PutDouble (const char *key, double value, const char *section, int dec = 2);

  /// Delete a key
  bool DeleteKey (const char *key, const char *section);

  /// Delete an entire section
  bool DeleteSection (const char *section);

  /// Copy all keys from one section to another.
  bool CopySection (const Profile& from_file, const char *from_sect, const char *to_sect=NULL);

  /// Return \b true if profile contains a non empty section with the given name
  bool HasSection (const char *section);

  /// Retrieve names of all keys in a section.
  int GetKeys (char *buffer, int sz, const char *section);

  /// Return the names of all sections in the INI file.
  int GetSections (char *sects, int sz);

private:

  char filename[MAX_PATH];
  bool temp_file;
};


#ifdef MLIBSPACE
}
#endif

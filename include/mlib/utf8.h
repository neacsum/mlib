/*!
  \file UTF8.H - UTF8 Conversion functions
  (c) Mircea Neacsu 2014-2017

  \addtogroup utf8
*/
#pragma once

#include <string>
#include <vector>

namespace utf8 {

std::string narrow (const wchar_t* s);
std::string narrow (const std::wstring& s);
std::string narrow (const std::u32string& s);

std::wstring widen (const char* s);
std::wstring widen (const std::string& s);
std::u32string runes (const std::string& s);

size_t length (const std::string& s);
bool next (const std::string& s, std::string::const_iterator& p);

char32_t rune (std::string::const_iterator p);
bool valid (const std::string& s);
bool valid (const char *s);

std::vector<std::string> get_argv ();
char** get_argv (int *argc);
void free_argv (int argc, char **argv);

bool mkdir (const char* dirname);
bool mkdir (const std::string& dirname);

bool rmdir (const char* dirname);
bool rmdir (const std::string& dirname);

bool chdir (const char* dirname);
bool chdir (const std::string& dirname);

bool chmod (const char* filename, int mode);
bool chmod (const std::string& filename, int mode);

std::string getcwd ();

bool access (const char* filename, int mode);
bool access (const std::string& filename, int mode);

bool remove (const char* filename);
bool remove (const std::string& filename);

bool rename (const char* oldname, const char* newname);
bool rename (const std::string& oldname, const std::string& newname);

void splitpath (const char* path, char* drive, char* dir, char* fname, char* ext);
void splitpath (const char* path, std::string& drive, std::string& dir, std::string& fname, std::string& ext);

inline
bool valid (const std::string& s)
{
  return valid (s.c_str ());
}

}

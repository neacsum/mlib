/*!
  \file UTF8.H - UTF8 Conversion functions
  (c) Mircea Neacsu 2014-2017

  \addtogroup other
*/
#pragma once

#ifndef _STRING_
#include <string>
#endif

#ifndef _VECTOR_
#include <vector>
#endif
namespace utf8 {

std::string narrow (const wchar_t* s);
std::wstring widen (const char* s);
std::string narrow (const std::wstring& s);
std::wstring widen (const std::string& s);

char **get_argv (int *argc);
void free_argv (int argc, char **argv);

int mkdir (const char* dirname);
int mkdir (const std::string& dirname);

int rmdir (const char* dirname);
int rmdir (const std::string& dirname);

int chdir (const char* dirname);
int chdir (const std::string& dirname);

std::string getcwd ();

int access (const char* path, int mode);
int access (const std::string& path, int mode);

int remove (const char* path);
int remove (const std::string& path);

int rename (const char* oldname, const char* newname);
int rename (const std::string& oldname, const std::string& newname);

void splitpath (const char* path, char* drive, char* dir, char* fname, char* ext);
void splitpath (const char* path, std::string& drive, std::string& dir, std::string& fname, std::string& ext);

std::vector<std::string> get_utf8argv ();

}

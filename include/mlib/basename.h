#pragma once
/*!
  \file BASENAME.H - Definitions for Unix-like basename() and dirname() functions.
*/

///Return a pointer to pathname of the file deleting any trailing '\\' character.
char *dirname (const char *filename);


///Return a pointer to the filename without any path component.
char *basename (const char* filename);

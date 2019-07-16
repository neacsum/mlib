#pragma once
/*!
  \file dprintf.cpp Definition of dprintf() function

  (c) Mircea Neacsu 1999-2000. All rights reserved.
*/

#define MAX_DPRINTF_CHARS 1024  ///< maximum message size
bool dprintf (const char *fmt, ... );

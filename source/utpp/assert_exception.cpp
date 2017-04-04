/*!
  \file assert_exception.cpp - Implementation of AssetException class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/

#include <utpp/assert_exception.h>
#include <cstring>

namespace UnitTest {

/// Constructor
AssertException::AssertException (const char* description_, const char* filename_, int lineNumber_)
  : line (lineNumber_)
{
  strcpy (description, description_);
  strcpy (file, filename_);
}

/// Return error message
char const* AssertException::what () const
{
  return description;
}

/// Return filename
char const* AssertException::filename () const
{
  return file;
}

/// Return line number
int AssertException::line_number () const
{
  return line;
}

/// Throws an AssetException
void ReportAssert (char const* description, char const* filename, int lineNumber)
{
  throw AssertException (description, filename, lineNumber);
}

}

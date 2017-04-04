#pragma once
/*!
  \file assert_exception.h - Definition of AssetException class

  (c) Mircea Neacsu 2017
  See README file for full copyright information.
*/
#include <exception>


namespace UnitTest {

/*!
  Exception thrown by ReportAssert function.
*/
class AssertException : public std::exception
{
public:
  AssertException (const char* description, const char* filename, int lineNumber);

  const char* what () const;

  const char* filename () const;
  int line_number () const;

private:
  char description[512];
  char file[256];
  int line;
};

void ReportAssert (char const* description, char const* filename, int lineNumber);

}

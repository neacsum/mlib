#pragma once
#include <exception>


namespace UnitTest {

/*!
  Exception thrown by ReportAssert function.
*/
class AssertException : public std::exception
{
public:
  AssertException (const char* description, const char* filename, int lineNumber);
  virtual ~AssertException ();

  const char* what () const;

  const char* Filename () const;
  int LineNumber () const;

private:
  char description[512];
  char filename[256];
  int lineNumber;
};

}

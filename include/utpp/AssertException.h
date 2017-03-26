#pragma once
#include <exception>


namespace UnitTest {

class AssertException : public std::exception
{
public:
  AssertException (char const* description, char const* filename, int lineNumber);
  virtual ~AssertException ();

  virtual char const* what () const;

  char const* Filename () const;
  int LineNumber () const;

private:
  char description[512];
  char filename[256];
  int lineNumber;
};

}

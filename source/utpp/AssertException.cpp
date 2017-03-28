#include <utpp/AssertException.h>
#include <cstring>

namespace UnitTest {

AssertException::AssertException (const char* description_, const char* filename_, int lineNumber_)
  : lineNumber (lineNumber_)
{
  strcpy (description, description_);
  strcpy (filename, filename_);
}

AssertException::~AssertException ()
{
}

char const* AssertException::what () const
{
  return description;
}

char const* AssertException::Filename () const
{
  return filename;
}

int AssertException::LineNumber () const
{
  return lineNumber;
}

}

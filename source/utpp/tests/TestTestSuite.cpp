#include <utpp/utpp.h>

// We're really testing if it's possible to use the same suite in two files
// to compile and link successfully (TestTestMacros.cpp has suite with the same name)
SUITE (SameTestSuite)
{
  TEST (DummyTest2)
  {
  }
}


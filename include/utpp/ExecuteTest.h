#pragma once

#include "TestDetails.h"
#include "MemoryOutStream.h"
#include "AssertException.h"
#include "CurrentTest.h"


namespace UnitTest {

template< typename T >
void ExecuteTest (T& testObject, TestDetails& details)
{
  CurrentTest.Details = &details;

  try
  {
    testObject.RunImpl ();
  }
  catch (AssertException const& e)
  {
    CurrentTest.Results->OnTestFailure (
      TestDetails (details.testName, details.suiteName, e.Filename (), e.LineNumber ()), e.what ());
  }
  catch (std::exception const& e)
  {
    MemoryOutStream stream;
    stream << "Unhandled exception: " << e.what ();
    CurrentTest.Results->OnTestFailure (details, stream.GetText ());
  }
  catch (...)
  {
    CurrentTest.Results->OnTestFailure (details, "Unhandled exception: Crash!");
  }
}

}

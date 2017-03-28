This test library is based on UnitTest++. See below for README notes of UnitTest++.

# Architecture #
## Overview ##
In it's simplest form, a test is defined using the `TEST` macro similarly with a
standard function call:
``````
TEST (MyFirstTest)
{
  // test code goes here
}
``````
A number of things happen behind the scenes when TEST macro is invoked:
1. It defines a class called "TestMyFirstTest" derived from
_Test_ class. The new class has a method called _RunImpl_ and the block of code
following the TEST macro becomes the body of the _RunImpl_ method.
2. It creates a ListAdder object (with the name 'adderMyFirstTest').
3. The ListAdder constructor inserts the newly created element in a linked list
whose head is returned by `Test::GetTestList()` static method.

The main program contains a call to _RunAllTests ()_ triggering the following 
sequence of events:
1. _RunAllTests_ function creates a _TestRunner_ object using a _TestReporter_ 
as a means for sending out test results and calls its' _RunTests()_ method.
2. _TestRunner::RunTests()_ iterates through the linked list mentioned before and,
for each object in the list calls the _TestRunner::Run_ method passing it the
the current _Test_ object.
3. Essentially, the _TestRunner::Run_ function, calls the _Test::Run_ function but
it also does a bit of housekeeping before and after to take care of things like
time measurement and results reporting.
4. _Test::Run_ wraps the RunImpl() function in a try...catch block and finally
calls this function. This is actually the test code that was placed after the
TEST macro.

## Checking Test Results ##
There are a a number of macro-definitions for testing abnormal conditions while
running a test here is a list of them:

CHECK(value)                   | Verify that value is true (or not 0)
CHECK_EQUAL (expected, actual) | Compare two values for equality
CHECK_CLOSE (expected, actual, tolerance) | Check that two values are closer than specified tolerance
CHECK_ARRAY_EQUAL (expected, actual, count) | Compare two arrays for equality
CHECK_ARRAY_CLOSE (expected, actual, count, tolerance) | Check that two arrays are closer than specified tolerance
CHECK_ARRAY2D_CLOSE (expected, actual, rows, columns, tolerance) | Check that two matrices are within the specified tolerance
CHECK_THROW (expression, ExceptionType) | Verifies that expression throws a given exception
CHECK_ASSERT (expression) | Verifies that expression throws an AssertException.

AssertException objects are thrown by _ReportAssert_ function.

## Using Test Suites ##


# UnitTest++ README #
Version: v1.4
Last update: 2008-10-30

UnitTest++ is free software. You may copy, distribute, and modify it under
the terms of the License contained in the file COPYING distributed
with this package. This license is the same as the MIT/X Consortium
license.

See src/tests/TestUnitTest++.cpp for usage.

Authors:
Noel Llopis (llopis@convexhull.com) 
Charles Nicholson (charles.nicholson@gmail.com)

Contributors:
Jim Tilander
Kim Grasman
Jonathan Jansson
Dirck Blaskey
Rory Driscoll
Dan Lind
Matt Kimmel -- Submitted with permission from Blue Fang Games
Anthony Moralez
Jeff Dixon
Randy Coulman
Lieven van der Heide

Release notes:
--------------
Version 1.4 (2008-10-30)
- CHECK macros work at arbitrary stack depth from inside TESTs.
- Remove obsolete TEST_UTILITY macros
- Predicated test execution (via TestRunner::RunTestsIf)
- Better exception handling for fixture ctors/dtors.
- VC6/7/8/9 support

Version 1.3 (2007-4-22)
- Removed dynamic memory allocations (other than streams)
- MinGW support
- Consistent (native) line endings
- Minor bug fixing

Version 1.2 (2006-10-29)
- First pass at documentation.
- More detailed error crash catching in fixtures.
- Standard streams used for printing objects under check. This should allow the
  use of standard class types such as std::string or other custom classes with
  stream operators to ostream.
- Standard streams can be optionally compiled off by defining UNITTEST_USE_CUSTOM_STREAMS
  in Config.h
- Added named test suites
- Added CHECK_ARRAY2D_CLOSE 
- Posix library name is libUnitTest++.a now
- Floating point numbers are postfixed with f in the failure reports

Version 1.1 (2006-04-18)
- CHECK macros do not have side effects even if one of the parameters changes state
- Removed CHECK_ARRAY_EQUAL (too similar to CHECK_ARRAY_CLOSE)
- Added local and global time constraints
- Removed dependencies on strstream
- Improved Posix signal to exception translator
- Failing tests are added to Visual Studio's error list
- Fixed Visual Studio projects to work with spaces in directories

Version 1.0 (2006-03-15)
- Initial release



# Architecture #

## Terminology ##
The library allows you to define test cases (called _tests_) and group those cases
in _test suites_. The test suites are executed and the results are displayed using
a _test repoter_.

Included are two test reporters: one that generates output to stdout (the default one)
and another that generates results in an XML file with a structure similar to the
files creatred by NUnit.

Throughout the execution of a test, one can verify the results and compare them with
expected results using _CHECK macros_.

## Overview ##
In its simplest form, a test is defined using the `TEST` macro similarly with a
standard function call:
``````
TEST (MyFirstTest)
{
  // test code goes here
}
``````
A number of things happen behind the scenes when TEST macro is invoked:
1. It defines a class called _TestMyFirstTest_ derived from
`Test` class. The new class has a method called `RunImpl` and the block of code
following the TEST macro becomes the body of the _RunImpl_ method.

2. It creates a small function (called _MyFirstTestmaker_) with the following body:
``````
Test* MyFirstTestmaker ()
{
  return new MyFirstTest;
}
``````

3. A pointer to this function together with some additional information is used
to create a `SuiteAdder` object (with the name _adderMyFirstTest_).

4. The `SuiteAdder` constructor inserts the newly created object in a container
maintained by a `SuitesList` object. There is only one instance of the SuitesList
object and this instance is returned by the GetSuitesList() function.

The main program contains a call to `RunAllTests()` that triggers the following 
sequence of events:
1. One of the parameters to the RunAllTests() function is a TestReporter object,
either one explicitly created or a default reporter that sends all results to stdout.

2. The RunAllTests() function calls `SuitesList::RunAll()` function.

3. `SuitesList::RunAll()` iterates through the list of tests mentioned before and,
for each object calls the maker function to instantiate a new Test-derived object
(like TestMyFirstTest).

4. It than calls the Test::Run method which in trun calls the TestMyFirstTest::RunImpl.
This is actually the test code that was placed after the TEST macro.

5. When the test has finished, the Test-derived object is deleted and  Essentially, the _TestRunner::Run_ function, calls the _Test::Run_ function but
it also does a bit of housekeeping before and after to take care of things like
time measurement and results reporting.

6. _Test::Run_ wraps the RunImpl() function in a try...catch block and finally
calls this function. This is actually the test code that was placed after the
TEST macro.

## Checking Test Results ##
There are a a number of macro-definitions for testing abnormal conditions while
running a test here is a list of them:

`CHECK(value)`  Verify that value is true (or not 0) 

`CHECK_EQUAL (expected, actual)` Compare two values for equality

`CHECK_CLOSE (expected, actual, tolerance)` Check that two values are closer than
 specified tolerance
 
`CHECK_ARRAY_EQUAL (expected, actual, count)` Compare two arrays for equality

`CHECK_ARRAY_CLOSE (expected, actual, count, tolerance)` Check that two arrays
 are closer than specified tolerance
 
`CHECK_ARRAY2D_CLOSE (expected, actual, rows, columns, tolerance)` Check that
 two matrices are within the specified tolerance
 
`CHECK_THROW (expression, ExceptionType)` Verifies that expression throws a given exception
`CHECK_ASSERT (expression)` Verifies that expression throws an AssertException. 

AssertException objects are thrown by _ReportAssert_ function.

## Using Test Suites ##
Tests can be grouped together in _suites_ like in the following example:
``````
SUITE (BigSuite)
{
  TEST (MyFirstTest)
  {
    // test code goes here
  }
  TEST (MySecondTest)
  {
    //another test case
  }
  // ....
}
``````
The SUITE macro defines in effect a namespace (called _SuiteBigSuite_) and makes
all tests, objects inside the namespace.

## Global Objects ##
There are two main global object pointers: `CurrentTest` and `CurrentReporter`.
They need to be global to be able to call check macros from anywhere. When the
test runner object is initialized (in the RunAllTests function) it also initializes
the CurrentReporter pointer.

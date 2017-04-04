#pragma once

#include <utpp/checks.h>
#include <utpp/assert_exception.h>
#include <utpp/test_reporter.h>

#ifdef CHECK
    #error UnitTest++ redefines CHECK
#endif

#ifdef CHECK_EQUAL
	#error UnitTest++ redefines CHECK_EQUAL
#endif

#ifdef CHECK_CLOSE
	#error UnitTest++ redefines CHECK_CLOSE
#endif

#ifdef CHECK_ARRAY_EQUAL
	#error UnitTest++ redefines CHECK_ARRAY_EQUAL
#endif

#ifdef CHECK_ARRAY_CLOSE
	#error UnitTest++ redefines CHECK_ARRAY_CLOSE
#endif

#ifdef CHECK_ARRAY2D_CLOSE
	#error UnitTest++ redefines CHECK_ARRAY2D_CLOSE
#endif

///Generate a failure if value is 0. Failure message is the value itself.
#define CHECK(value)                                                          \
  do                                                                          \
  {                                                                           \
    try {                                                                     \
      if (!UnitTest::Check(value))                                            \
      {                                                                       \
        ReportFailure (__FILE__, __LINE__, "Check failed: " #value);          \
      }                                                                       \
    }                                                                         \
    catch (...) {                                                             \
      ReportFailure (__FILE__, __LINE__,                                      \
        "Unhandled exception in CHECK(" #value ")");                          \
    }                                                                         \
  } while (0)

///Generate a failure with the given message if value is 0
#define CHECK_EX(value, message)                                              \
  do                                                                          \
  {                                                                           \
    try {                                                                     \
      if (!UnitTest::Check(value))                                            \
        ReportFailure (__FILE__, __LINE__, message);                          \
    }                                                                         \
    catch (...) {                                                             \
      ReportFailure (__FILE__, __LINE__,                                      \
        "Unhandled exception in CHECK(" #value ")");                          \
    }                                                                         \
  } while (0)

///Generate a failure if actual value is different than expected
#define CHECK_EQUAL(expected, actual)                                         \
  do                                                                          \
  {                                                                           \
    try {                                                                     \
      std::string msg;                                                        \
      if (!UnitTest::CheckEqual(expected, actual, msg))                       \
        ReportFailure (__FILE__, __LINE__, msg);                              \
    }                                                                         \
    catch (...) {                                                             \
      ReportFailure (__FILE__, __LINE__,                                      \
        "Unhandled exception in CHECK_EQUAL(" #expected ", " #actual ")");    \
    }                                                                         \
  } while (0)


#define CHECK_CLOSE(expected, actual, tolerance)                              \
  do                                                                          \
  {                                                                           \
    try {                                                                     \
      std::string msg;                                                        \
      if (!UnitTest::CheckClose (expected, actual, tolerance, msg))           \
        ReportFailure (__FILE__, __LINE__, msg);                              \
    }                                                                         \
    catch (...) {                                                             \
      ReportFailure (__FILE__, __LINE__,                                      \
        "Unhandled exception in CHECK_CLOSE(" #expected ", " #actual ")");    \
    }                                                                         \
  } while (0)

#define CHECK_ARRAY_EQUAL(expected, actual, count) \
  do                                                                          \
  {                                                                           \
    try {                                                                     \
      std::string msg;                                                        \
      if (!UnitTest::CheckArrayEqual (expected, actual, count, msg))          \
        ReportFailure (__FILE__, __LINE__, msg);                              \
    }                                                                         \
    catch (...) {                                                             \
      ReportFailure (__FILE__, __LINE__,                                      \
       "Unhandled exception in CHECK_ARRAY_EQUAL(" #expected ", " #actual ")"); \
    }                                                                         \
  } while (0)

#define CHECK_ARRAY_CLOSE(expected, actual, count, tolerance)                 \
  do                                                                          \
  {                                                                           \
    try {                                                                     \
      std::string msg;                                                        \
      if (!UnitTest::CheckArrayClose (expected, actual, count, tolerance, msg)) \
        ReportFailure (__FILE__, __LINE__, msg);                              \
    }                                                                         \
    catch (...) {                                                             \
      ReportFailure (__FILE__, __LINE__,                                      \
        "Unhandled exception in CHECK_ARRAY_CLOSE(" #expected ", " #actual ")"); \
    }                                                                         \
  } while (0)

#define CHECK_ARRAY2D_CLOSE(expected, actual, rows, columns, tolerance) \
  do                                                                          \
  {                                                                           \
    try {                                                                     \
      std::string msg;                                                        \
      if (!UnitTest::CheckArray2DClose (expected, actual, rows, columns, tolerance, msg)) \
        ReportFailure (__FILE__, __LINE__, msg);                              \
    }                                                                         \
    catch (...) {                                                             \
      ReportFailure (__FILE__, __LINE__,                                      \
        "Unhandled exception in CHECK_ARRAY2D_CLOSE(" #expected ", " #actual ")"); \
    }                                                                         \
  } while (0)


#define CHECK_THROW(expression, ExpectedExceptionType) \
  do                                                                          \
  {                                                                           \
    bool caught_ = false;                                                     \
    try { expression; }                                                       \
    catch (const ExpectedExceptionType&) { caught_ = true; }                  \
    catch (...) {}                                                            \
    if (!caught_)                                                             \
      ReportFailure (__FILE__, __LINE__,                                      \
        "Expected exception: \"" #ExpectedExceptionType "\" not thrown");     \
  } while(0)

#define CHECK_THROW_EQUAL(expression, ExpectedExceptionType, expected) \
  do                                                                          \
    {                                                                         \
    bool caught_ = false;                                                     \
    try { expression; }                                                       \
    catch (const ExpectedExceptionType& actual) {                             \
      caught_ = true;                                                         \
      std::string msg;                                                        \
      if (!UnitTest::CheckEqual(expected, actual, msg))                       \
        ReportFailure (__FILE__, __LINE__, msg);                              \
    }                                                                         \
    catch (...) {}                                                            \
    if (!caught_)                                                             \
      ReportFailure (__FILE__, __LINE__,                                      \
        "Expected exception: \"" #ExpectedExceptionType "\" not thrown");     \
    } while(0)
#define CHECK_ASSERT(expression) \
    CHECK_THROW(expression, UnitTest::AssertException);


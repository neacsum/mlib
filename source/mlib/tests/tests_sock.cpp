#include <utpp/utpp.h>
#include <mlib/wsockstream.h>

using namespace mlib;


TEST (connect_timeout)
{
  int timeout = 3;
  UNITTEST_TIME_CONSTRAINT (timeout * 1000 + 100);

  sock a (SOCK_STREAM);
  inaddr nonexistent ("198.51.100.1", 80);
  int result = a.connect (nonexistent, timeout);
  CHECK_EQUAL (WSAETIMEDOUT, result);
};
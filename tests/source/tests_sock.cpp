#include <utpp/utpp.h>
#include <mlib/wsockstream.h>
#include <mlib/thread.h>

using namespace mlib;

SUITE (sockets)
{
TEST (connect_timeout)
{
  int timeout = 3; //timeout in seconds
  UTPP_TIME_CONSTRAINT (timeout * 1000 + 100);

  sock a (SOCK_STREAM);
  inaddr nonexistent ("198.51.100.1", 80);
  auto result = a.connect (nonexistent, timeout);
  result.facility ().log (result);
  CHECK_EQUAL (WSAETIMEDOUT, result);
}

TEST (inaddr_basic1)
{
  inaddr lh ("localhost", 1234);

  CHECK_EQUAL (INADDR_LOOPBACK, lh.host ());
}

TEST (inaddr_assignment)
{
  inaddr lh ("localhost", 1234);
  inaddr addr2;
  addr2 = lh;
  CHECK_EQUAL (INADDR_LOOPBACK, addr2.host ());
  CHECK_EQUAL (1234, addr2.port ());
}

TEST (inaddr_copy)
{
  inaddr lh ("localhost", 1234);
  inaddr addr3 (lh);
  CHECK_EQUAL (INADDR_LOOPBACK, addr3.host ());
  CHECK_EQUAL (1234, addr3.port ());
  CHECK_EQUAL ("127.0.0.1", addr3.ntoa ());
}

TEST (inaddr_dns_fail_1)
{
  inaddr noname ("127.0.0.2", 1234);
  CHECK_EQUAL ("127.0.0.2", noname.hostname ());
}

TEST (inaddr_dns_fail_2)
{
  int ret = 0;
  try {
    inaddr nohost ("test.invalid", 1234);
  }catch (erc& e) {
    ret = e;
  }
  CHECK_EQUAL (WSAHOST_NOT_FOUND, ret);
}

TEST (inaddr_dns_fail_3)
{
  inaddr x;
  auto e = x.host ("test.invalid");
  CHECK_EQUAL (WSAHOST_NOT_FOUND, e);
}

TEST (dgram_send_receive)
{
  event go;
  char buf[80];
  auto f = [&]() -> int {
    sock s (SOCK_DGRAM);
    s.connect (inaddr ("127.0.0.1", 1234));
    go.wait ();
    s.sendto (inaddr ("127.0.0.2", 1234), "TEST", 5);
    return 0;
  };

  auto g = [&] () -> int {
    sock s (SOCK_DGRAM);
    s.bind (inaddr ("127.0.0.2", 1234));
    s.recv (buf, sizeof(buf));
    return 0;
  };

  thread th1 (f), th2(g);
  buf[0] = 0;
  th1.start ();
  th2.start ();
  go.signal ();
  wait_all ({&th1, &th2});

  CHECK_EQUAL ("TEST", buf);
}

TEST (sock_readready)
{
  event sent;
  auto f = [&] () -> int {
    sock s (SOCK_DGRAM);
    s.connect (inaddr ("127.0.0.1", 1234));
    s.sendto (inaddr ("127.0.0.2", 1234), "TEST", 5);
    sent.signal ();
    return 0;
  };

  thread th (f);
  sock s (SOCK_DGRAM);
  s.bind (inaddr ("127.0.0.2", 1234));
  CHECK (!s.is_readready (0));

  th.start ();
  sent.wait ();
  CHECK (s.is_readready (0));
}

TEST (stream_send_receive)
{
  event th2_go, th2_done;
  unsigned short port;
  char buf[80];
  auto f = [&] () -> int {
    sock s(SOCK_STREAM);
    s.bind (inaddr("127.0.0.1",0));
    port = s.name ().port ();
    s.listen ();
    th2_go.signal ();
    sockstream ss (s.accept ());
    ss << "TEST STREAM" << std::endl;
    th2_done.wait ();
    return 0;
  };

  auto g = [&] () -> int {
    sockstream ss (SOCK_STREAM);
    th2_go.wait ();
    ss->connect (inaddr("127.0.0.1", port));
    ss >> buf;
    th2_done.signal ();
    return 0;
  };

  thread th1 (f), th2 (g);
  buf[0] = 0;
  th1.start ();
  th2.start ();

  wait_all ({&th1, &th2});
  CHECK_EQUAL ("TEST", buf);
}

} //end suite 

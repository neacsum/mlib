#include <utpp/utpp.h>
#include <mlib/mlib.h>
#include <string>
#pragma hdrstop

using namespace mlib;
using namespace std::literals::string_literals;

SUITE (sockets)
{

TEST (sock_ctor_invalid)
{
  sock s;
  CHECK (!s.is_open ());
  CHECK_EQUAL (INVALID_HANDLE_VALUE, s.handle ());
}

TEST (sock_ctor_valid)
{
  sock s(SOCK_DGRAM);
  CHECK (s.is_open ());
  EXPECT_NE (INVALID_HANDLE_VALUE, s.handle ());
}

TEST (sock_ctor_copy)
{
  sock s (SOCK_DGRAM);
  sock s1 (s);
  EXPECT_EQ (s1.handle (), s.handle ());
}

TEST (sock_ctor_move)
{
  sock s (SOCK_DGRAM);
  HANDLE sh = s.handle ();
  sock s1 = std::move(s);
  EXPECT_EQ (s1.handle (), sh);
  EXPECT_EQ (INVALID_HANDLE_VALUE, s.handle ());
}

TEST (sock_assign)
{
  sock s (SOCK_DGRAM);
  sock s1;

  s1 = s;
  EXPECT_EQ (s1.handle (), s.handle ());
}

TEST (sock_move_assign)
{
  sock s (SOCK_DGRAM);
  HANDLE sh = s.handle ();
  sock s1;

  s1 = std::move (s);
  EXPECT_EQ (s1.handle (), sh);
  CHECK (!s.is_open ());
}

TEST (sock_open)
{
  sock s1 (SOCK_DGRAM), s2;
  CHECK (s1.is_open () && !s2.is_open ());

  s2 = s1;
  CHECK_EQUAL (s1.handle (), s2.handle ());
  CHECK_EQUAL (s1, s2); //socket equality operator

  //opening an opened socket receives a new handle
  s2.open (SOCK_DGRAM);
  CHECK (s2.is_open ());
  CHECK (s1.handle () != s2.handle ());
  CHECK (s1 != s2);
}

TEST (sock_close)
{
  sock s1 (SOCK_DGRAM), s2;
  CHECK (s1.is_open () && !s2.is_open ());

  s2 = s1;
  CHECK_EQUAL (s1.handle (), s2.handle ());

  SOCKET h1 = (SOCKET)s1.handle ();

  s1.close ();
  CHECK (!s1.is_open ());
  CHECK (s2.is_open ());

  CHECK_EQUAL (erc::success, s1.close ()); // closing a closed socket is harmless
  
  s2.close ();
  CHECK (!s2.is_open ());
  CHECK_EQUAL (SOCKET_ERROR, closesocket (h1));
  CHECK_EQUAL (WSAENOTSOCK, WSAGetLastError ());
}

TEST (sock_send_string)
{
  sock s (SOCK_STREAM), c1 (SOCK_STREAM), c2;
  s.bind (inaddr ("localhost", 0));
  s.listen ();

  c1.connect (*s.name());
  s.accept (c2);
  c1.send ("TEST"s);

  char buf[80];
  auto l = c2.recv (buf, sizeof (buf));
  buf[l] = 0; //make received data a C string

  CHECK_EQUAL ("TEST", buf);

  wchar_t wbuf[80];
  c1.send (L"TEST"s);
  l = c2.recv (wbuf, sizeof (wbuf));
  wbuf[l / sizeof (wchar_t)] = 0;
  CHECK_EQUAL (L"TEST", wbuf);
}


TEST (connect_timeout)
{
  int timeout = 3; //timeout in seconds
  UTPP_TIME_CONSTRAINT (timeout * 1000 + 100);

  sock a (SOCK_STREAM);
  inaddr nonexistent ("198.51.100.1", 80); //per RFC5737 
  auto result = a.connect (nonexistent, timeout);
  CHECK_EQUAL (WSAETIMEDOUT, result);
}

TEST (accept_timeout)
{
  sock s(SOCK_STREAM), cl;
  s.bind (inaddr ("localhost", 0));
  s.listen ();
  auto result = s.accept (cl, 1);
  CHECK_EQUAL (WSAETIMEDOUT, result);
}

TEST (accept_ok)
{
  sock s (SOCK_STREAM), c1 (SOCK_STREAM), c2;
  s.bind (inaddr ("localhost", 0));
  s.listen ();

  c1.connect (*s.name());

  CHECK_EQUAL (erc::success, s.accept (c2));
  CHECK (c2.is_open ());
}

TEST (accept_ok2)
{
  sock s (SOCK_STREAM), c1 (SOCK_STREAM), c2;
  s.bind (inaddr ("localhost", 0));
  s.listen ();

  c1.connect (*s.name());

  CHECK_EQUAL (erc::success, s.accept (c2, 0));
  CHECK (c2.is_open ());
}

TEST (timeout_values)
{
  sock s (SOCK_STREAM);

  int to = 3;
  s.recvtimeout (to);
  s.sendtimeout (to+1);

  CHECK_EQUAL (to, s.recvtimeout ());
  CHECK_EQUAL (to + 1, s.sendtimeout ());
}

TEST (sock_type)
{
  sock s1 (SOCK_STREAM);
  sock s2 (SOCK_DGRAM);
  sock s3 (SOCK_RAW, AF_INET, IPPROTO_ICMP);

  CHECK_EQUAL (SOCK_STREAM, s1.gettype());
  CHECK_EQUAL (SOCK_DGRAM, s2.gettype ());
  CHECK_EQUAL (SOCK_RAW, s3.gettype ());
}

TEST (inaddr_basic1)
{
  inaddr lh ("localhost", 1234);
  CHECK_EQUAL (INADDR_LOOPBACK, lh.host ());
}

TEST (inaddr_bcast)
{
  inaddr lh ("255.255.255.255", 0);
  CHECK_EQUAL (INADDR_BROADCAST, lh.host ());
 }

TEST (inaddr_port_string)
{
  inaddr lh ("localhost", "1234");
  CHECK_EQUAL (1234, lh.port ());

  auto err = lh.port ("http");
  CHECK_EQUAL (erc::success, err);
  CHECK_EQUAL (80, lh.port ());
  
  err = lh.port ("blah");
  CHECK_EQUAL (WSANO_DATA, err);
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
  auto e = x.host ("test.invalid"); //per RFC2606
  CHECK_EQUAL (WSAHOST_NOT_FOUND, e);
}

TEST (dgram_send_receive)
{
  auto_event go;
  char buf[80];
  auto f = [&]() -> int {
    sock s (SOCK_DGRAM);
    go.wait ();
    std::cout << "Sending datagram...";
    s.sendto (inaddr ("127.0.0.2", 1234), "TEST", 5);
    return 0;
  };

  auto g = [&] () -> int {
    sock s (SOCK_DGRAM);
    s.bind (inaddr ("127.0.0.2", 1234));
    inaddr sender;
    s.recvfrom (sender, buf, sizeof(buf));
    std::cout << "... Datagram received from " << sender << std::endl;
    return 0;
  };

  thread th1 (f), th2(g);
  buf[0] = 0;
  th1.start ();
  th2.start ();
  go.signal ();
  auto ret = wait_all ({&th1, &th2}, 2000);
  CHECK (ret < WAIT_OBJECT_0 + 2);
  CHECK_EQUAL ("TEST", buf);
}

TEST (dgram_send_string)
{
  sock s1 (SOCK_DGRAM), s2 (SOCK_DGRAM);
  s1.connect ({"127.0.0.1", 1234});
  s2.bind ({"127.0.0.2", 1234});

  char buf[80];
  inaddr actual_sender;
  inaddr expected_sender = *s1.name ();

  s1.sendto (inaddr("127.0.0.2", 1234), "TEST"s);
  auto l = s2.recvfrom (actual_sender, buf, sizeof (buf));
  buf[l] = 0;

  CHECK_EQUAL ("TEST", buf);
  CHECK_EQUAL (expected_sender, actual_sender);
}

TEST (sock_readready)
{
  auto_event sent;
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

// This is a liberal transcription of the sample code from recv documentation
// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-recv
TEST (sample_recv)
{
  sock ConnectSocket (SOCK_STREAM), BindSocket (SOCK_STREAM), AcceptSocket(SOCK_STREAM);
  constexpr int DEFAULT_BUFLEN = 512, DEFAULT_PORT = 27015;

  const char *sendbuf = "this is a test";
  char recvbuf[DEFAULT_BUFLEN];
  inaddr service{"127.0.0.1", DEFAULT_PORT};

  try {
    BindSocket.bind (service);
    BindSocket.listen ();
    ConnectSocket.connect (service);
    BindSocket.accept (AcceptSocket);

    int iResult = (int)ConnectSocket.send (sendbuf, strlen (sendbuf));
    printf ("Bytes Sent: %ld\n", iResult);

    ConnectSocket.shutdown (sock::shut_write);

    do
    {
      iResult = (int)AcceptSocket.recv (recvbuf, DEFAULT_BUFLEN);
      if (iResult > 0)
        printf ("Bytes received: %d\n", iResult);
      else if (iResult == 0)
        printf ("Connection closed\n");
    } while (iResult > 0);

  }
  catch (erc &x) {
    ABORT_EX (0, "Error : %d - %s", (int)x, x.message().c_str());
  }
  
}

TEST (stream_send_receive)
{
  unsigned short port;
  char buf[80];
  auto f = [&] () -> int {
    sock s(SOCK_STREAM);
    s.bind (inaddr("127.0.0.1",0));
    port = s.name ()->port();
    s.listen ();
    inaddr who;

    sock client;
    s.accept (client, &who);
    std::cout << "Incoming connection from " << who << std::endl;
    sockstream ss (client);
    ss << "TEST STREAM" << std::endl;
    Sleep (1000);
    return 0;
  };

  auto g = [&] () -> int {
    sockstream ss (SOCK_STREAM);
    Sleep (1000);
    ss->connect (inaddr("127.0.0.1", port));
    ss.getline (buf, sizeof(buf));
    return 0;
  };

  thread th1 (f), th2 (g);
  buf[0] = 0;
  th1.start ();
  th2.start ();

  try
  {
    auto ret = wait_all ({&th1, &th2}, 4000);
    std::cout << "wait_all return=" << ret << std::endl;
    CHECK (ret < WAIT_OBJECT_0 + 2);
  }
  catch (erc& x)
  {
    ABORT_EX (0, "Error : %d - %s", (int)x, x.message().c_str());
  }

  CHECK_EQUAL ("TEST STREAM", buf);
}

#if 0
TEST (tcp_server_maxconn)
{
  tcpserver srv ("test server", 2);
  srv.socket ().bind (inaddr ("localhost", 0));
  auto serv_port = srv.socket ().name ().port ();
  srv.set_connfunc ([](auto &c) {
    sockstream strm (c);
    strm << "Hello" << std::endl;
    Sleep(1000);
    return 0;
  });
  srv.start ();
  bool caught = false;
  sockstream clients[3];

  for (int i = 0; i < 3; i++)
  {
    clients[i]->open (SOCK_STREAM);
    clients[i]->connect (inaddr ("127.0.0.1", serv_port));
    std::string s;
    clients[i] >> s;
  }
  try
  {
    sock ss (SOCK_STREAM);
    ss.connect (inaddr ("127.0.0.1", serv_port));  
  }
  catch (erc &x)
  {
    std::cout << "test server Connection failed - " << x.message () << std::endl;
    caught = true;
  }

  CHECK (caught);
  for (int i = 0; i < 3; i++)
    clients[i]->close ();
}
#endif

} //end suite 

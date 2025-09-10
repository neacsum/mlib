#include <utpp/utpp.h>
#include <mlib/mlib.h>
#pragma hdrstop
#include <fstream>
#include <utils.h>

using namespace mlib;

using namespace std;

namespace mlib::http {
struct UnitTest_connection : public connection
{
  UnitTest_connection (sock& s, server& srv)
    : connection (s, srv) {};

public:
  connection::method_;
  connection::path_;
  connection::iheaders;
  connection::do_auth;
};

SUITE (HttpServer)
{
TEST (uri_handler)
{
  http::server srv;
  srv.socket ().bind (inaddr (INADDR_LOOPBACK, 12345));
  srv.add_handler ("/?A", [] (http::connection& cl, void*) -> int { 
    cl.serve_buffer ("Hello world!");
    return HTTP_OK;
   });
  srv.start ();
  int status_code;
  string uri;
  auto fn = [&] () -> int {
    sockstream ws (inaddr ("127.0.0.1", srv.socket ().name ()->port ()));
    ws << "GET " << uri << " HTTP/1.1" << endl;
    ws << "Host: " << to_string (*srv.socket ().name ()) << endl;
    ws << "Connection: Close" << endl;
    ws << endl << flush;
    char text[1024];
    ws.getline (text, sizeof (text));
    cout << "Response:" << endl;
    cout << text << endl;
    strtok (text, " ");
    status_code = atoi (strtok (NULL, " "));
    while (!ws.eof ())
    {
      ws.getline (text, sizeof (text));
      cout << text << endl;
    }

    return 1;
  };
  try
  {
    thread cl1 (fn);
    uri = "/%3fA";
    cl1.start ();
    cl1.wait ();
    CHECK_EQUAL (200, status_code);

    thread cl2 (fn);
    uri = "/%3fB";
    cl2.start ();
    cl2.wait ();
    CHECK_EQUAL (404, status_code);
  }
  catch (erc& x)
  {
    cout << "erc=" << (int)x << " at " << x.facility ().name () << endl;
    CHECK_EX (!x, "erc=%d at %s", (int)x, x.facility ().name().c_str());
  }

  srv.terminate ();
}

TEST (url_decode_ok)
{
  string str = "key1=value1&key2=hello%20world%21";
  http::str_pairs pairs;

  auto ret = internal::parse_urlparams (str, pairs);
  CHECK (ret);
  CHECK_EQUAL (2, pairs.size ());
}

TEST (url_decode_bad)
{
  string str = "key1=value1&key2";
  http::str_pairs pairs;

  auto ret = internal::parse_urlparams (str, pairs);
  CHECK (!ret);
}

TEST (POST_ok)
{
  http::server srv;
  srv.socket ().bind (inaddr (INADDR_LOOPBACK, 12345));

  srv.start ();

  int stat;
  char text[1024];

  auto client = thread ([&] () -> int {
    sockstream ws (inaddr (INADDR_LOOPBACK, 12345));
    ws << "POST / HTTP/1.1" << endl
       << "Host: 127.0.0.1:12345" << endl
       << "Content-Length: 10" << endl
       << endl
       << "0123456789"
       << flush;
    ws->shutdown (sock::shut_write);
    Sleep (100);
    ws.getline (text, sizeof (text));
    cout << "Status: " << text << endl;
    char* ptr = strchr (text, ' ');
    stat = ptr ? atoi (ptr) : -1;

    while (!ws.eof ())
    {
      ws.getline (text, sizeof (text));
      cout << text << endl;
    }
    return 1;
  });
  client.start ();
  client.wait (1000);

  CHECK_EQUAL (204, stat);

  srv.terminate ();
}

TEST (POST_invalid_content_length)
{
  http::server srv;
  srv.socket ().bind (inaddr (INADDR_LOOPBACK, 12345));

  srv.start ();

  int stat;
  char text[1024];

  auto client = thread ([&] () -> int {
    sockstream ws (inaddr (INADDR_LOOPBACK, 12345));
    ws << "POST / HTTP/1.1" << endl
       << "Host: 127.0.0.1:12345" << endl
       << "Content-Length: -10" << endl
       << endl
       << "0123456789" << flush;
    ws->shutdown (sock::shut_write);
    Sleep (100);
    ws.getline (text, sizeof (text));
    cout << "Status: " << text << endl;
    char* ptr = strchr (text, ' ');
    stat = ptr ? atoi (ptr) : -1;

    while (!ws.eof ())
    {
      ws.getline (text, sizeof (text));
      cout << text << endl;
    }
    return 1;
  });
  client.start ();
  client.wait (1000);

  CHECK_EQUAL (400, stat);

  srv.terminate ();
}

TEST (POST_no_content_length)
{
  http::server srv;
  srv.socket ().bind (inaddr (INADDR_LOOPBACK, 12345));

  srv.start ();

  int stat;
  char text[1024];

  auto client = thread ([&] () -> int {
    sockstream ws (inaddr (INADDR_LOOPBACK, 12345));
    ws << "POST / HTTP/1.1" << endl
       << "Host: 127.0.0.1:12345" << endl
       << endl
       << "0123456789" << flush;
    ws->shutdown (sock::shut_write);
    Sleep (100);
    ws.getline (text, sizeof (text));
    cout << "Status: " << text << endl;
    char* ptr = strchr (text, ' ');
    stat = ptr ? atoi (ptr) : -1;

    while (!ws.eof ())
    {
      ws.getline (text, sizeof (text));
      cout << text << endl;
    }
    return 1;
  });
  client.start ();
  client.wait (1000);

  CHECK_EQUAL (400, stat);

  srv.terminate ();
}


TEST (BadHeader1)
{
  http::server srv;
  srv.socket ().bind (inaddr (INADDR_LOOPBACK, 12345));

  auto fname = srv.docroot ();
  fname += "/index.html";
  ofstream idx (fname);
  idx << "<html><head><title>TEST Bad Headers</title></head><body>Some stuff</body></html>\r\n";
  idx.close ();
  srv.start ();

  int stat;
  char text[1024];

  auto client = thread ([&] () -> int {
    sockstream ws (inaddr (INADDR_LOOPBACK, 12345));
    ws << "GET / HTTP/1.1" << endl 
      << "Host : 127.0.0.1:12345" << endl //white space in field name
      << endl << flush;
    ws->shutdown (sock::shut_write);
    Sleep (100);
    ws.getline (text, sizeof (text));
    cout << "Status: " << text << endl;
    char* ptr = strchr (text, ' ');
    stat = ptr ? atoi (ptr) : -1;

    while (!ws.eof ())
    {
      ws.getline (text, sizeof (text));
      cout << text << endl;
    }
    return 1;
  });
  client.start ();
  client.wait (1000);
  srv.terminate ();
  remove (fname);

  CHECK_EQUAL (400, stat);
}

TEST (BadHeader2)
{
  http::server srv;
  srv.socket ().bind (inaddr (INADDR_LOOPBACK, 12345));

  auto fname = srv.docroot ();
  fname += "/index.html";
  ofstream idx (fname);
  idx << "<html><head><title>TEST Bad Headers</title></head><body>Some stuff</body></html>\r\n";
  idx.close ();
  srv.start ();

  int stat;
  char text[1024];

  auto client = thread ([&] () -> int {
    sockstream ws (inaddr (INADDR_LOOPBACK, 12345));
    ws << "GET / HTTP/1.1" << endl
       << endl // missing "Host" header
       << endl
       << flush;
    ws->shutdown (sock::shut_write);
    Sleep (100);
    ws.getline (text, sizeof (text));
    cout << "Status: " << text << endl;
    char* ptr = strchr (text, ' ');
    stat = ptr ? atoi (ptr) : -1;

    while (!ws.eof ())
    {
      ws.getline (text, sizeof (text));
      cout << text << endl;
    }
    return 1;
  });
  client.start ();
  client.wait (1000);
  srv.terminate ();
  remove (fname);

  CHECK_EQUAL (400, stat);
}

TEST (BadHeader3)
{
  http::server srv;
  srv.socket ().bind (inaddr (INADDR_LOOPBACK, 12345));

  auto fname = srv.docroot ();
  fname += "/index.html";
  ofstream idx (fname);
  idx << "<html><head><title>TEST Bad Headers</title></head><body>Some stuff</body></html>\r\n";
  idx.close ();
  srv.start ();

  int stat;
  char text[1024];

  auto client = thread ([&] () -> int {
    sockstream ws (inaddr (INADDR_LOOPBACK, 12345));
    ws << "GET / HTTP/1.1" << endl
       << "Host: 127.0.0.1:12345" << endl 
       << "Host: 127.0.0.2:12345" << endl //multiple "Host" headers
       << endl
       << flush;
    ws->shutdown (sock::shut_write);
    Sleep (100);
    ws.getline (text, sizeof (text));
    cout << "Status: " << text << endl;
    char* ptr = strchr (text, ' ');
    stat = ptr ? atoi (ptr) : -1;

    while (!ws.eof ())
    {
      ws.getline (text, sizeof (text));
      cout << text << endl;
    }
    return 1;
  });
  client.start ();
  client.wait (1000);
  srv.terminate ();
  remove (fname);

  CHECK_EQUAL (400, stat);
}


TEST (Binding)
{
  http::server srv;
  srv.socket ().bind (inaddr (INADDR_LOOPBACK, 12345));

  int port = srv.socket ().name ()->port ();
  CHECK_EQUAL (12345, port);

  ofstream idx ("index.html");
  idx << "<html><head><title>TEST Address Binding</title></head><body>Some stuff</body></html>\r\n";
  idx.close ();
  srv.docroot (".");
  srv.start ();
  auto client = thread ([&] () -> int {
    sockstream ws (inaddr (INADDR_LOOPBACK, port));
    ws << "GET / HTTP/1.0" << endl << endl << flush;
    ws->shutdown (sock::shut_write);
    Sleep (100);
    char text[1024];
    cout << "Response:" << endl;
    while (!ws.eof ())
    {
      ws.getline (text, sizeof (text));
      cout << text << endl;
    }

    return 1;
  });
  client.start ();
  CHECK_EQUAL (WAIT_OBJECT_0, client.wait (1000));
  srv.terminate ();
  remove ("index.html");
}

TEST (Auth)
{
  http::server srv;
  srv.add_secured_path ("Control", "/ctl");
  srv.add_user ("Control", "admin", "admin");
  srv.add_user ("Control", "Alice", "password");

  bool ok = srv.verify_authorization ("Control", "admin", "admin");
  CHECK (ok);

  ok = srv.verify_authorization ("Control", "Alice", "password");
  CHECK (ok);

  ok = srv.verify_authorization ("Control", "Eve", "nopass");
  CHECK (!ok);
}

TEST (AuthMatch)
{
  http::server srv;
  string realm;
  srv.add_secured_path ("All", "/");
  srv.add_secured_path ("Control", "/ctl");
  srv.add_secured_path ("Control1", "/ctl/inner");
  bool ok = srv.is_protected ("/status/map.html", realm);
  CHECK (ok);
  CHECK_EQUAL ("All", realm);
  ok = srv.is_protected ("/ctl/change.cgi", realm);
  CHECK (ok);
  CHECK_EQUAL ("Control", realm);
  ok = srv.is_protected ("/ctl/inner/admin.cgi", realm);
  CHECK (ok);
  CHECK_EQUAL ("Control1", realm);
  ok = srv.is_protected ("/ctl/inner/deep/stuff.html", realm);
  CHECK (ok);
  CHECK_EQUAL ("Control1", realm);
}


}

class HttpServerFixture
{
public:
  HttpServerFixture ();
  ~HttpServerFixture ();
  http::server srv;
  ofstream idx;
  string request;
  string uri;
  string answer;
  std::function<int ()> cfunc;
  int status_code;
};

HttpServerFixture::HttpServerFixture ()
  : uri ("/")
{
  idx.open ("index.html");
  idx << "<html><head><title>TEST Page</title></head><body>Some stuff</body></html>\r\n";
  idx.close ();
  srv.docroot (".");
  srv.start ();
  cfunc = [&] () -> int {
    sockstream ws (inaddr ("127.0.0.1", srv.socket ().name ()->port ()));
    ws << "GET " << uri << " HTTP/1.1" << endl;
    ws << "Host: " << to_string(*srv.socket().name()) << endl; 
    ws << request.c_str () << endl << flush;
    ws->shutdown (sock::shut_write);
    Sleep (100);
    char text[1024];
    ws.getline (text, sizeof (text));
    cout << "Response:" << endl;
    cout << text << endl;
    strtok (text, " ");
    status_code = atoi (strtok (NULL, " "));
    while (!ws.eof ())
    {
      ws.getline (text, sizeof (text));
      answer += text;
      answer += '\n';
      cout << text << endl;
    }
    cout << "End of response" << endl;
    return 1;
  };
}

HttpServerFixture::~HttpServerFixture ()
{
  remove ("index.html");
}

TEST_FIXTURE (HttpServerFixture, OkAnswer)
{
  mlib::thread client (cfunc);
  Sleep (1000);
  client.start ();
  CHECK (client.wait (2000) != WAIT_TIMEOUT);
  CHECK_EQUAL (200, status_code);
  srv.terminate ();
}

TEST_FIXTURE (HttpServerFixture, Answer404)
{
  uri = "/no_such_thing";

  mlib::thread client (cfunc);
  client.start ();
  CHECK (client.wait (200) != WAIT_TIMEOUT);
  CHECK_EQUAL (404, status_code);
  srv.terminate ();
}

TEST_FIXTURE (HttpServerFixture, Answer401)
{
  srv.add_secured_path ("Control", "/");

  mlib::thread client (cfunc);
  client.start ();
  CHECK (client.wait (200) != WAIT_TIMEOUT);
  CHECK_EQUAL (401, status_code);
  srv.terminate ();
}

TEST_FIXTURE (HttpServerFixture, AuthOk)
{
  srv.add_secured_path ("Control", "/");
  srv.add_user ("Control", "Alice", "password");
  request = "Authorization: Basic QWxpY2U6cGFzc3dvcmQ=\r\n";

  mlib::thread client (cfunc);
  client.start ();
  CHECK (client.wait (200) != WAIT_TIMEOUT);
  CHECK_EQUAL (200, status_code);
  srv.terminate ();
}

TEST_FIXTURE (HttpServerFixture, HttpBadPassword)
{
  srv.add_secured_path ("Control", "/");
  srv.add_user ("Control", "Alice", "password");
  request = "Authorization: Basic QWxpY2U6d3Jvbmc=\r\n"; // wrong password

  mlib::thread client (cfunc);
  client.start ();
  CHECK (client.wait (200) != WAIT_TIMEOUT);
  CHECK_EQUAL (401, status_code);
  srv.terminate ();
}

} //end suite

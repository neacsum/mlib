#include <utpp/utpp.h>
#include <mlib/httpd.h>
#include <mlib/wsockstream.h>
#include <fstream>
#include <mlib/trace.h>

#ifdef MLIBSPACE
using namespace MLIBSPACE;
#endif

using namespace std;

TEST (Auth)
{
  httpd srv (8080);
  srv.add_realm ("Control", "/ctl");
  srv.add_user ("Control", "admin", "admin");
  srv.add_user ("Control", "Alice", "password");

  bool ok = srv.authenticate ("Control", "admin", "admin");
  CHECK (ok);

  ok = srv.authenticate ("Control", "Alice", "password");
  CHECK (ok);

  ok = srv.authenticate ("Control", "Eve", "nopass");
  CHECK (!ok);
}

TEST (AuthMatch)
{
  httpd srv (8080);
  string realm;
  srv.add_realm ("Control", "/ctl");
  srv.add_realm ("Control1", "/ctl/inner");
  bool ok = srv.is_protected ("/status/map.html", realm);
  CHECK(!ok);
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

class HttpServerFixture {
public:
  HttpServerFixture ();
  ~HttpServerFixture ();
  httpd srv;
  ofstream idx;
  string request;
  string uri;
  string answer;
  std::function<int(void*)> cfunc;
  int status_code;
};

HttpServerFixture::HttpServerFixture () :
  srv (8080),
  uri ("/")
{
  idx.open("index.html");
  idx << "<html><head><title>TEST Page</title></head><body>Some stuff</body></html>\r\n";
  idx.close ();
  srv.docroot (".");
  srv.start ();
  cfunc = [&] (void*) -> int {
    sockstream ws(inaddr ("127.0.0.1", 8080));
    ws << "GET " << uri << " HTTP/1.1" << endl;
    TRACE ("GET %s", uri.c_str ());
    ws << request.c_str () << endl << flush;
    ws->shutdown (sock::shut_write);
    Sleep (100);
    char text[1024];
    ws.getline (text, sizeof (text));
    TRACE ("Status line %s", text);
    strtok (text, " ");
    status_code = atoi (strtok(NULL, " "));
    while (!ws.eof ())
    {
      ws.getline (text, sizeof (text));
      answer += text;
      answer += '\n';
      TRACE ("Received line %s", text);
    }

    return 1;
  };
}

HttpServerFixture::~HttpServerFixture ()
{
  srv.terminate ();
  remove ("index.html");
}


TEST_FIXTURE (HttpServerFixture, OkAnswer)
{
  thread client (cfunc, 0, "HTTPclient");
  client.start ();
  client.wait ();
  CHECK_EQUAL (200, status_code);
}

TEST_FIXTURE (HttpServerFixture, Answer404)
{
  uri = "no_such_thing";

  thread client (cfunc, 0, "HTTPclient");
  client.start ();
  client.wait ();
  CHECK_EQUAL (404, status_code);
}

TEST_FIXTURE (HttpServerFixture, Answer401)
{
  srv.add_realm ("Control", "/");

  thread client (cfunc, 0, "HTTPclient");
  client.start ();
  client.wait ();
  CHECK_EQUAL (401, status_code);
}

TEST_FIXTURE (HttpServerFixture, AuthOk)
{
  srv.add_realm ("Control", "/");
  srv.add_user ("Control", "Alice", "password");
  request = "Authorization: Basic QWxpY2U6cGFzc3dvcmQ=\r\n";

  thread client (cfunc, 0, "HTTPclient");
  client.start ();
  client.wait ();
  CHECK_EQUAL (200, status_code);
}

TEST_FIXTURE (HttpServerFixture, HttpBadPassword)
{
  srv.add_realm ("Control", "/");
  srv.add_user ("Control", "Alice", "password");
  request = "Authorization: Basic QWxpY2U6d3Jvbmc=\r\n"; //wrong password

  thread client (cfunc, 0, "HTTPclient");
  client.start ();
  client.wait ();
  CHECK_EQUAL (401, status_code);
}

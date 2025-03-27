/*
  http_auth.cpp - Authentication with HTTP server

  (c) Mircea Neacsu 2025

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/

#include <iostream>
#include <fstream>

#include <mlib/mlib.h>
#include <utf8/utf8.h>

using namespace mlib;
using namespace std;

#define HOME_PAGE "index.shtml"
#define PAGE2     "eve.shtml"

const char* index_shtml = R"(<html>
<head>
  <title>Auth UI</title>
</head>
<body>
  Hello <!--#echo var="user" -->
<p>
Known users are "Alice" and "Bob"<br/>
Authorization header is <!--#echo var="auth" -->
</body>
</html>
)";

const char* eve_shtml = R"(
<head>
  <title>Auth UI - Eve's Secret page</title>
</head>
<body>
  Hello <!--#echo var="user" -->
<p>
Only Eve can access this page<br/>
Authorization header is <!--#echo var="auth" -->
</body>
</html>
)";


int main ()
{
  // variable updated by HTML user interface
  string user;
  string auth;


  // create HTTP server
  http::server ui_server;
  ui_server.default_uri (HOME_PAGE);

  ui_server.add_var ("user", &user);
  ui_server.add_var ("auth", &auth);

  // save HTML page to a file
  ofstream f (ui_server.docroot () / HOME_PAGE);
  f << index_shtml;
  f.close ();

  // save second page to a file
  f.open (ui_server.docroot () / PAGE2);
  f << eve_shtml;
  f.close ();

  ui_server.add_secured_path ("index", "/");
  ui_server.add_user ("index", "Alice", "alpha");
  ui_server.add_user ("index", "Bob", "beta");

  ui_server.add_secured_path ("secret", PAGE2);
  ui_server.add_user ("secret", "Eve", "epsilon");

  ui_server.add_handler (HOME_PAGE, [&] (http::connection& client, void*) -> int {
    user = client.get_auth_user ();
    auth = client.get_ihdr ("authorization");
    return HTTP_CONTINUE;
  });
  ui_server.add_handler (PAGE2, [&] (http::connection& client, void*) -> int {
    user = client.get_auth_user ();
    auth = client.get_ihdr ("authorization");
    return HTTP_CONTINUE;
  });

  // Start HTTP server
  ui_server.socket ().bind (inaddr(INADDR_LOOPBACK, 8080));
  ui_server.start ();
  Sleep (10);
  if (!ui_server.is_running ())
  {
    std::cout << "Failed to start HTTP server!!\n";
    return 1;
  }

  // Direct a browser to HTML page
  utf8::ShellExecute ("http://"s + to_string (*ui_server.socket ().name ()));


  std::cout << "Server is running. " << std::endl
            << "Users and passwords are: Alice/alpha, Bob/beta" << std::endl
            << "Eve/epsilon can go to eve.shtml" << std::endl <<std::endl
            << "Press ENTER to stop" << std::endl;
  char x;
  std::cin.get (x);

  // stop server and clean up
  ui_server.terminate ();
  std::filesystem::remove (ui_server.docroot () / HOME_PAGE);
  std::filesystem::remove (ui_server.docroot () / PAGE2);

  return 0;
}

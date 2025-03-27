/*
  ui-echo.cpp - A minimalistic user interface using HTML

  (c) Mircea Neacsu 2022

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


  This is a very simple application that shows in a browser a form with a
  single text input field. When user presses the "OK" button the application
  receives the updated text and displays it on the console window.

  It shows how to use the SSI mechanism of the HTTP server.
*/

#include <iostream>
#include <fstream>

#include <mlib/mlib.h>
#include <utf8/utf8.h>

using namespace mlib;

using namespace std;

#pragma comment (lib, "utf8.lib")

#define HOME_PAGE "index.shtml"

const char* index_shtml = R"(<html>
<head>
  <title>Echo UI</title>
  <script>
    function load() {
      document.getElementById('field').value = "<!--#echo var="text" -->"
    }
  </script>
</head>
<body onload="load() ">
  <form method="post" action="/uivars">
    Text: <input name="text" id="field" size="80"/>
    <input type="submit" value="OK" />
  </form>
  Type 'quit' to exit.
<p>
Update counter: <!--#echo var="counter" --><br/>
</body>
</html>
)";


int main()
{
  // variable updated by HTML user interface
  string field{"Hello world!"};

  int counter = 0;
  mlib::manual_event ok_clicked;

  // create HTTP server
  http::server ui_server;

  ui_server.add_var ("counter", &counter);
  ui_server.add_var ("text", &field);

  auto fname = ui_server.docroot() / HOME_PAGE;
  // save HTML page to a file
  ofstream idx (fname.string ());
  idx << index_shtml;
  idx.close ();


  // when receiving a POST message, echo the field, then reload page
  ui_server.add_post_handler ("/uivars",
    [&] (http::connection& cl, void*) -> int {
      /* The server does not update any variable in response to a POST request. 
      User has to retrieve any variable from the request body.*/
      if (cl.has_bparam ("text"))
        field = cl.get_bparam ("text");
      cout << "Web page says: " << field << endl;
      ++counter;
      cl.redirect (string ("/") + HOME_PAGE);
      ok_clicked.signal ();
      return 1;
    });

  // Start HTTP server
  ui_server.start ();
  Sleep (10);
  if (!ui_server.is_running ())
  {
    std::cout << "Failed to start HTTP server!!\n";
    return 1;
  }

  // Direct a browser to HTML page
  utf8::ShellExecute ("http://"s + to_string(*ui_server.socket ().name ()) 
    + "/" HOME_PAGE);

  // wait until user types "QUIT"
  while (_stricmp (field.c_str(), "quit"))
    ok_clicked.wait();

  // stop server and clean up
  ui_server.terminate ();
  std::filesystem::remove (fname);

  return 0;
}

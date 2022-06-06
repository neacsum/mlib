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

  It shows how to create a JSON bridge and attach it to a HTTP server.
*/

#include <iostream>
#include <fstream>

#include <mlib/jbridge.h>
#include <utf8/winutf8.h>

using namespace mlib;
using namespace std;

#pragma comment (lib, "utf8.lib")

const char* page1 = R"(<html>
<head>
  <title>Echo UI</title>
  <script>
let xhr = new XMLHttpRequest();
xhr.open("GET", "/uivars?field");
xhr.onload = function() {
  if (xhr.status == 200)
    document.getElementById('field').value = JSON.parse(xhr.response);
  else
    alert(`Error ${xhr.status}: ${xhr.statusText}`);
};
xhr.send (); 
  </script>
</head>
<body>
  <form method="post" action="/uivars">
    Text: <input name="field" id="field" size="80"/>
    <input type="submit" value="OK" />
  </form>
  Type 'quit' to exit.
</body>
</html>
)";

//variable updated by HTML user interface
char field[80] = {"Hello world!"};

int main()
{
  // save HTML page to a file
  ofstream idx ("index.html");
  idx << page1;
  idx.close ();

  // create HTTP server and set it serve pages from current directory
  httpd ui_server;
  ui_server.docroot (".");

  // create user interface and link it to server
  JSONBridge ui ("uivars");
  ui.attach_to (ui_server);
  ui.add_var (field, "field");

  // when receiving a POST message, echo the field, then reload page
  ui.set_action ([](JSONBridge& ui) {
    cout << "Web page says: " << field << endl;
    ui.client ()->redirect ("/");
    });

  // Start HTTP server
  ui_server.start ();
  while (!ui_server.is_running ())
    Sleep (0);

  // Direct a browser to HTML page
  utf8::ShellExecute ("http://localhost:" + to_string(ui_server.port()));

  // wait until user types "QUIT"
  while (_stricmp (field, "quit"))
    Sleep (100);

  // stop server and clean up
  ui_server.terminate ();
  remove ("index.html");
}

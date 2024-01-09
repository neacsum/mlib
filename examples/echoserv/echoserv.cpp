/*
  ECHOSERV.CPP - A simple TCP server using mlib::tcpserver class.

  The program starts an echo server on port 12321. The server will echo back any
  line received from a client. To terminate the program press any key.

  The MIT License (MIT)

  (c) Mircea Neacsu 2017-2020

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

#include <mlib/tcpserv.h>
#include <conio.h>

using namespace std;
using namespace mlib;

#pragma comment (lib, "mlib")

tcpserver srv (0, "Echo server", 2);

int main (int argc, char** argv)
{

  // This is a demo server,  bind listening socket only on the loop-back interface.
  // A real world server would probably use INADDR_ANY
  srv.socket ().bind (inaddr (INADDR_LOOPBACK, 12321));


  srv.set_connfunc (
    [](sock conn)->int {
      sockstream strm (conn);
      std::string line;
      inaddr other;
      conn.peer (other);
      cout << "Connection from " << other << " socket " << conn.handle() << endl;

      //echo each line
      while (getline (strm, line))
        strm << line << endl;

      cout << "Terminated connection to " << other << " socket " << conn.handle () << endl;
      return 0;
    }
  );

  srv.start ();
  Sleep (10);

  if (srv.get_state() != thread::state::running)
  {
    cout << "Could not start echo server. Error was " << srv.socket ().clearerror () << endl;
    srv.terminate ();
    return 1;
  }
  inaddr me;
  srv.socket ().name (me);
  cout << "Echo server waiting for connections on " << me << endl;
  cout << "Timeout is " << srv.timeout () << " seconds." << endl;
  cout << "Press any key to exit..." << endl;

  while (_kbhit ())
    ;
  _getch ();
  
  srv.terminate ();
  return 0;
}


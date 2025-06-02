/*
  ECHOCLIENT.CPP - A simple client for echo server using socket streams.

  Command line:
    echoclient [<host>:<port>]

  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.

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

#include <mlib/mlib.h>
#include <conio.h>
#include <string>

int main (int argc, char** argv)
{
  std::string peer{"localhost"};
  unsigned short port = 12321;

  if (argc > 1)
  {
    char* sep = strchr (argv[1], ':');
    if (sep)
    {
      port = (unsigned short)atoi (sep + 1);
      *sep = 0;
    }
    peer = argv[1];
  }
  mlib::inaddr srv (peer, port);
  std::cout << "Connecting to " << srv << std::endl;
  std::cout << "Type CTRL-Z and ENTER to exit\n\n";
  try
  {
    mlib::sockstream strm (srv);
    while (1)
    {
      std::string outgoing, incoming;

      std::getline (std::cin, outgoing);
      if (std::cin.eof ())
        break;
      strm << outgoing << std::endl;
      getline (strm, incoming);
      std::cout << incoming << std::endl;
    }
  }
  catch (mlib::erc& err)
  {
    std::cout << err.facility ().name () << " - " << err.message () << std::endl;
  }
}
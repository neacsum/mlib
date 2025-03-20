/*
  HTTP Server Demo
  Shows how to attach user handlers to different URIs.

  This example has three handlers:
  - http://localhost:8080/hi - invokes the `say_hello` handler
  - http://localhost:8080/hi/tom invokes `say_hello_tom` handler. Note how a
    more specific handler ("hi/tom") overrides a more generic one ("/hi")
  - http://localhost:8080/headers returns the curent request and response
    headers.
*/
#include <mlib/mlib.h>

using namespace mlib;

int say_hello (http::connection& client, void*)
{
  client.serve_buffer ("Hello world!");
  return HTTP_OK;
}

int say_hello_tom (http::connection& client, void*)
{
  client.serve_buffer ("Hello Tom! Glad to see you.");
  return HTTP_OK;
}

int show_headers (http::connection& client, void*)
{
  std::stringstream strm;
  strm << "Request headers:\n" 
    << client.get_request_headers () << std::endl; 
  strm << "Response headers (partial):\n" 
    << client.get_response_headers () << std::endl;
  client.serve_buffer (strm.str());
  return HTTP_OK;
}


int main (int argc, char** argv)
{
  http::server my_server;
  my_server.socket ().bind (inaddr (INADDR_LOOPBACK, 8080));
  my_server.add_handler ("/hi", say_hello, nullptr);
  my_server.add_handler ("/hi/tom", say_hello_tom, nullptr);
  my_server.add_handler ("/headers", show_headers, nullptr);
  my_server.start ();

  std::cout << "Server is running. Connect to http://localhost:8080/hi" << std::endl
            << "Press ENTER to stop" << std::endl;

  char x;
  std::cin.get (x);
  return 0;
}

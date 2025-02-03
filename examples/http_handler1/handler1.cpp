#include <mlib/mlib.h>

using namespace mlib;

int say_hello (http::connection& client, void*)
{
  client.serve_buffer ("Hello world!");
  return HTTP_OK;
}

int main (int argc, char** argv)
{
  http::server my_server (8080);
  my_server.add_handler ("/", say_hello, nullptr);
  my_server.start ();

  std::cout << "Server is running. Connect to http://localhost:8080" << std::endl
            << "Press ENTER to stop" << std::endl;

  char x;
  std::cin.get(x);
  return 0;
}

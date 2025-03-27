#include <mlib/mlib.h>
#include <filesystem>
#include <fstream>

using namespace mlib;
using namespace std;

int say_hello (http::connection& client, void*)
{
  client.serve_buffer ("Hello world!");
  return HTTP_OK;
}

const char* index_shtml = R"(<html>
<head>
  <title>Handler 1</title>
</head>
<body>
  This is the <b>index.html</b> page.
  <p>You can also go to <a href="/hi">Hello World</a> page.</p>
</body>
</html>
)";

int main (int argc, char** argv)
{
  http::server my_server;

  auto fname = my_server.docroot () / "index.html";
  // save HTML page to a file
  ofstream idx (fname);
  idx << index_shtml;
  idx.close ();

  my_server.socket ().bind (inaddr (INADDR_LOOPBACK, 8080));
  my_server.add_handler ("/hi", say_hello, nullptr);
  my_server.start ();

  std::cout << "Server is running. Connect to http://localhost:8080" << std::endl
            << "Press ENTER to stop" << std::endl;

  char x;
  std::cin.get(x);
  std::filesystem::remove (fname);
  return 0;
}

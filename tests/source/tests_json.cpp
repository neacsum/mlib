#include <utpp/utpp.h>
#include <mlib/json.h>

using namespace mlib;
using namespace std;

SUITE (Json)
{

TEST (NodeCreation)
{
  json::node n1 ("string");

  json::node n2 (json::object);
  n2["child"] = n1;
  n2["2nd child"] = 24;
  n2["3rd child"] = true;
  n2["4th child"] = json::node();

  json::node n3;
  n3["n2"] = n2;

  cout << n3 << endl;
}

}
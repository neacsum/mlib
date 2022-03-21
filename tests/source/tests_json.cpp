#include <utpp/utpp.h>
#include <mlib/json.h>

using namespace mlib;
using namespace std;

SUITE (Json)
{

TEST (ObjectNodeCreation)
{
  json::node n1 ("string");

  json::node n2;
  n2["child"] = n1;
  n2["2nd child"] = 24;
  n2["3rd child"] = true;
  n2["4th child"] = json::node();
  CHECK_EQUAL (4, n2.size ());

  json::node n3;
  n3["n2"] = n2;
  CHECK_EQUAL (1, n3.size ());


  cout << n3 << endl;
}

TEST (ArrayNodeCreation)
{
  json::node n1 ("string");

  json::node n2;
  n2[0] = n1;
  n2[1] = 24;
  // n2[2] will be null
  n2[3] = true;
  n2[4] = json::node ();
  CHECK_EQUAL (5, n2.size ());

  json::node n3;
  n3[0] = n2;
  CHECK_EQUAL (1, n3.size ());


  cout << n3 << endl;
}

TEST (StreamRead)
{
  string in = R"({
    "encoding" : "UTF-8",
    "plug-ins" : [
      "python",
        "c++",
        "ruby"
    ] ,
    "indent" : { "length" : 3, "use_space" : true }
})";
  
  stringstream ss (in);

  json::node n;
  ss >> n;
  cout << n;
}


}
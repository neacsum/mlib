#include <utpp/utpp.h>
#include <mlib/json.h>
#include <utf8/utf8.h>

#pragma warning (disable:4566)

using namespace mlib;
using namespace std;

SUITE (Json)
{

TEST (StringRead)
{
  json::node n;
  string in1 = R"({
    "encoding" : "UTF-8",
    "plug-ins" : [
      "python",
        "c++",
        "ruby"
    ] ,
    "indent" : { "length" : 3, "use_space" : true }})";
  CHECK_EQUAL (ERR_SUCCESS, n.read (in1));
}

TEST (StringWrite)
{
  string in{ R"({"asd":"sdf"})" };
  json::node n;
  n.read (in);
  string out;
  n.write (out);
  CHECK_EQUAL (in, out);
}

TEST (Write_with_quoted_strings)
{
  string in{ R"({"foo\u0000bar":42})" };
  json::node n;
  n.read (in);
  string out;
  n.write (out);
  CHECK_EQUAL (in, out);
}

TEST (quoted_string_outside_bmp)
{
  string in{ R"({"G clef":"\ud834\udd1e"})" };
  json::node n;
  n.read (in);
  string out;
  n.write (out);
  CHECK_EQUAL (in, out);
}

TEST (move_constructor)
{
  json::node n1;
  n1.read (R"({"asd":"sdf"})");
  json::node n2 = std::move (n1);
  CHECK_EQUAL (json::type::null, n1.kind ());
  CHECK_EQUAL ("sdf", n2["asd"].to_string());
}

TEST (move_assignment)
{
  json::node n1, n2;
  n1.read (R"({"asd":"sdf"})");
  n2 = std::move (n1);
  CHECK_EQUAL (json::type::null, n1.kind ());
  CHECK_EQUAL ("sdf", n2["asd"].to_string ());
}

TEST (equality)
{
  string in = R"({
    "encoding" : "UTF-8",
    "plug-ins" : [
      "python",
        "c++",
        "ruby"
    ] ,
    "indent" : { "length" : 3, "use_space" : true }})";
  json::node n1, n2;
  n1.read (in);
  n2.read (in);

  CHECK_EQUAL (n1, n2);
}

TEST (inequality)
{
  string in = R"({
    "encoding" : "UTF-8",
    "plug-ins" : [
      "python",
        "c++",
        "ruby"
    ] ,
    "indent" : { "length" : 3, "use_space" : true }})";
  json::node n1, n2;
  n1.read (in);
  n2.read (in);

  n2["indent"]["use_space"] = false;
  CHECK (n1 != n2);
}

TEST (string_in_supplemental_plane)
{
  //example from RFC 8259
  json::node n;
  string s = R"(["\ud834\udd1e"])";
  CHECK_EQUAL (ERR_SUCCESS, n.read(s));
  CHECK_EQUAL ((char32_t)0x1d11e, utf8::rune (n[0].to_string().c_str()));
}

/*
  test cases from https://github.com/nst/JSONTestSuite
  These are the y_... tests that need to pass
*/
TEST (y_tests)
{
  json::node n;
  //array arrays with spaces
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([[]   ])"));

  //array empty string
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([""])"));

  //array empty
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([])"));

  //array false
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([false])"));

  //array heterogeneous
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([null, 1, "1", {}])"));

  //array null
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([null])"));

  //array with 1 and newline
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([1
])"));

  //array with several null
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([1,null,null,null,2])"));

  //number
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([123e65])"));

  //number 0E+1
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([1e+0])"));

  //number 0e1
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([0e1])"));

  //number after space
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([ 4])"));

  //number double close to 0
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([-0.000000000000000000000000000000000000000000000000000000000000000000000000000001])"));

  //number with exp
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([20e1])"));

  //number minus 0
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([-0])"));
  CHECK_EQUAL (0, n[0].to_number ());

  //number negative int
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([-123])"));
  CHECK_EQUAL (-123, n[0].to_number ());

  //number positive int
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([123])"));
  CHECK_EQUAL (123, n[0].to_number ());

  //number simple real
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([123.456789])"));
  CHECK_EQUAL (123.456789, n[0].to_number ());

  //number real exponent
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([123e45])"));
  CHECK_EQUAL (123e45, n[0].to_number ());

  //number real fraction exponent
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([123.456e78])"));
  CHECK_EQUAL (123.456e78, n[0].to_number ());

  //number negative 1
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([-1])"));

  //number real capital E
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([1E22])"));
  CHECK_EQUAL (1e22, n[0].to_number ());

  // number real capital E negative exp
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([1E-2])"));
  CHECK_EQUAL (0.01, n[0].to_number ());

  //number real capital E positive exp
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([1E+2])"));
  CHECK_EQUAL (100, n[0].to_number ());

  //number real negative exp
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([1e-2])"));
  CHECK_EQUAL (0.01, n[0].to_number ());

  //number real exp with + sign
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"([1e+2])"));
  CHECK_EQUAL (100, n[0].to_number ());

  //object
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"({"asd":"sdf", "dfg":"fgh"})"));

  //object basic
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"({"asd":"sdf"})"));
  CHECK_EQUAL ("sdf", n["asd"].to_string ());

  //object escaped null in key
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"({"foo\u0000bar": 42})"));
  string foo_bar = { 'f','o','o','\0','b', 'a','r' };
  CHECK_EQUAL (42, n[foo_bar].to_number ());

  //object extreme numbers
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"({ "min": -1.0e+28, "max": 1.0e+28 })"));
  CHECK_EQUAL (-1e28, n["min"].to_number ());
  CHECK_EQUAL (1e28, n["max"].to_number ());

  //object long strings
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"({"x":[{"id": "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}], "id": "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"})"));
  CHECK_EQUAL (n["x"][0]["id"], n["id"]);

  //array object string unicode
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"({"title":"\u041f\u043e\u043b\u0442\u043e\u0440\u0430 \u0417\u0435\u043c\u043b\u0435\u043a\u043e\u043f\u0430" })"));
  CHECK_EQUAL (u8"Полтора Землекопа", n["title"].to_string ());

  //object with newlines
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"({
"a": "b"
})"));

  //string 1 2 3 UTF8 sequences
  CHECK_EQUAL (ERR_SUCCESS, n.read (R"(["\u0060\u012a\u12AB"])"));

  //string accepted surrogate pair
  CHECK_EQUAL (ERR_SUCCESS, n.read ("[\"\\uD801\\udc37\"]"));
  wstring ws = utf8::widen (n[0].to_string ());
  char32_t r = utf8::rune (n[0].to_string ().c_str ());
}

/*
  test cases from https://github.com/nst/JSONTestSuite
  These are the n_... tests that need to fail
*/

TEST (n_tests)
{
  json::node n;

  //array 1 true without comma
  CHECK_EQUAL (ERR_JSON_INPUT, n.read ("[1 true]"));

  //array incomplete
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"(["x")"));
  
  //array colon instead of comma
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"(["": 1])"));

  //array comma after close
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"([""],)"));
  
  //array comma and number
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"([,1])"));

  //array double comma
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"([1,,2])"));

  //array double extra comma
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"(["x",,])"));

  //array extra close
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"(["x"]])"));

  //array extra comma
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"(["",])"));

  //array incomplete
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"(["x")"));

  //array items separates by colon
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"([1:2])"));

  //array just comma
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"([,])"));

  //array just minus
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"([-])"));

  //array missing value
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"([   , ""])"));

  //array new
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"(["a",
4
,1,)"));

  //array number and comma
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"([1,])"));

  //array number and several commas
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"([1,,])"));

  //array spaces, vertical tab, formfeed
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"(["a"\f])"));

  //array star inside
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"([*])"));

  //array unclosed
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"(["")"));

  //array unclosed trailing comma
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"([1,)"));

  //array unclosed with newlines
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"([1,
1
,1
)"));

  //array unclosed with object inside
  CHECK_EQUAL (ERR_JSON_INPUT, n.read (R"([{})"));


}

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
  string in1 = R"({
    "encoding" : "UTF-8",
    "plug-ins" : [
      "python",
        "c++",
        "ruby"
    ] ,
    "indent" : { "length" : 3, "use_space" : true }
})";
 
  string in2 = R"|( {
      "elmType": "div",
      "txtContent" : {
      "operator": "floor",
        "operands" : [
      {
        "operator": "/",
          "operands" : [
        {
          "operator": "-",
            "operands" : [
          {
            "operator": "+",
              "operands" : [
                "=Number( 'Approval date')",
                {
                "operator": "*",
                "operands" : [
                "=Number( 'Approval validity period')",
                "365",
                "24",
                "60",
                "60",
                "1000"
                ]
                }
              ]
          },
            "=Number('@now')"
            ]
        },
          86400000
          ]
      }
        ]
    }
  }
  )|";

  stringstream ss;
  json::node n;

  ss.str (in2);

  ss >> n;
  cout << json::indent << n << endl << endl;

  cout << json::tabs;

  ss.str (in1);
  ss >> n;
  cout << n << endl << endl;

}


}
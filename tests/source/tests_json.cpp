#include <utpp/utpp.h>
#include <mlib/mlib.h>
#pragma hdrstop

#include <float.h>
#include <utf8/utf8.h>

#pragma warning (disable:4566)

#define STR(X) #X
#define STRINGIZE(X) STR(X)

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
  CHECK_EQUAL (erc::success, n.read (in1));
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

TEST (Write_fixed_manip)
{
  json::node n;
  n[0] = 123E9 + 1e-5;
  n[1] = 123E9 + 1e-5;

  stringstream ss;
  ss << fixed << setprecision(3) << n[0] << " " << defaultfloat << n[1];
  CHECK_EQUAL ("123000000000.000 1.23e+11", ss.str ());
}

TEST (Write_integers)
{
  json::node n;
  n[0] = 123E9;
  n[1] = -123E9;

  stringstream ss;
  ss << n[0] << " " << n[1];
  CHECK_EQUAL ("123000000000 -123000000000", ss.str ());
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
  CHECK_EQUAL ("sdf", static_cast<string>(n2["asd"]));
}

TEST (move_assignment)
{
  json::node n1, n2;
  n1.read (R"({"asd":"sdf"})");
  n2 = std::move (n1);
  CHECK_EQUAL (json::type::null, n1.kind ());
  CHECK_EQUAL ("sdf", static_cast<string>(n2["asd"]));
}

TEST (num_vector_assignment)
{
  std::vector<int> v{ 1, 2, 3 };
  json::node n;
  n = v;

  string out;
  n.write (out);
  CHECK_EQUAL ("[1,2,3]", out);
}

TEST (num_vector_constructor)
{
  std::vector<int> v{ 1, 2, 3 };
  json::node n (v);

  string out;
  n.write (out);
  CHECK_EQUAL ("[1,2,3]", out);
}

TEST (string_vector_assignment)
{
  std::vector<std::string> v{ "abc", "def", "ghi"};
  json::node n;
  n = v;

  string out;
  n.write (out);
  CHECK_EQUAL ("[\"abc\",\"def\",\"ghi\"]", out);
}

TEST (string_vector_constructor)
{
  std::vector<std::string> v{ "abc", "def", "ghi" };
  json::node n = v;

  string out;
  n.write (out);
  CHECK_EQUAL ("[\"abc\",\"def\",\"ghi\"]", out);
}

TEST (obj_to_json)
{
  struct Person {
    std::string name;
    int age;
    double height;

    inline int to_json (json::node& n) const {
      n["Name"] = name;
      n["Age"] = age;
      n["Height"] = height;
      return 1;
    }
  };

  Person p{ "Joe", 42, 1.78 };

  json::node n1, n2;
  n1 = p;
  n2["Name"] = p.name;
  n2["Age"] = p.age;
  n2["Height"] = p.height;

  CHECK_EQUAL (n2, n1);

  json::node n3 = p;
  CHECK_EQUAL (n2, n3);
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
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (0x1d11e, (unsigned int)utf8::rune (static_cast<const char*>(n[0])));
}
/*
  test cases from https://github.com/nst/JSONTestSuite
  These are the y_... tests that need to pass
*/
TEST (y_tests)
{
  json::node n;
  string s;
  //array arrays with spaces
  CHECK_EQUAL (erc::success, n.read (R"([[]   ])"));

  //array empty string
  CHECK_EQUAL (erc::success, n.read (R"([""])"));

  //array empty
  CHECK_EQUAL (erc::success, n.read (R"([])"));

  //array false
  CHECK_EQUAL (erc::success, n.read (R"([false])"));

  //array heterogeneous
  CHECK_EQUAL (erc::success, n.read (R"([null, 1, "1", {}])"));

  //array null
  CHECK_EQUAL (erc::success, n.read (R"([null])"));

  //array with 1 and newline
  CHECK_EQUAL (erc::success, n.read (R"([1
])"));

  //array with several null
  CHECK_EQUAL (erc::success, n.read (R"([1,null,null,null,2])"));

  //number
  CHECK_EQUAL (erc::success, n.read (R"([123e65])"));

  //number 0E+1
  CHECK_EQUAL (erc::success, n.read (R"([1e+0])"));

  //number 0e1
  CHECK_EQUAL (erc::success, n.read (R"([0e1])"));

  //number after space
  CHECK_EQUAL (erc::success, n.read (R"([ 4])"));

  //number double close to 0
  s = R"([-0.000000000000000000000000000000000000000000000000000000000000000000000000000001])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_CLOSE (-1E-78, static_cast<double>(n[0]), DBL_EPSILON);

  //Min positive value
  s = "[" STRINGIZE (DBL_MIN) "]";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_CLOSE (DBL_MIN, static_cast<double>(n[0]), DBL_EPSILON);

  //smallest such that 1.0+DBL_EPSILON != 1.0
  s = "[" STRINGIZE (DBL_EPSILON) "]";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK (1.0 != 1.0 + static_cast<double>(n[0]));

  //number with exp
  s = R"([20e1])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (200, static_cast<int>(n[0]));

  //number minus 0
  s = R"([-0])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (0, static_cast<int>(n[0]));

  //number negative int
  s = R"([-123])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (-123, static_cast<int>(n[0]));

  //number positive int
  s = R"([123])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (123, static_cast<int>(n[0]));

  //number simple real
  s = R"([123.456789])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (123.456789, static_cast<double>(n[0]));

  //number real exponent
  s = R"([123e45])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (123e45, static_cast<double>(n[0]));

  //number real fraction exponent
  s = R"([123.456e78])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (123.456e78, static_cast<double>(n[0]));

  //number negative 1
  s = R"([-1])";
  CHECK_EQUAL (erc::success, n.read (s));

  //number real capital E
  s = R"([1E22])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (1e22, static_cast<double>(n[0]));

  // number real capital E negative exp
  s = R"([1E-2])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (0.01, static_cast<double>(n[0]));

  //number real capital E positive exp
  s = R"([1E+2])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (100, static_cast<double>(n[0]));

  //number real negative exp
  s = R"([1e-2])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (0.01, static_cast<double>(n[0]));

  //number real exp with + sign
  s = R"([1e+2])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (100, static_cast<int>(n[0]));

  //object
  s = R"({"asd":"sdf", "dfg":"fgh"})";
  CHECK_EQUAL (erc::success, n.read (s));

  //object basic
  s = R"({"asd":"sdf"})";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL ("sdf", static_cast<string>(n["asd"]));

  //object escaped null in key
  s = R"({"foo\u0000bar": 42})";
  CHECK_EQUAL (erc::success, n.read (s));
  string foo_bar = { 'f','o','o','\0','b', 'a','r' };
  CHECK_EQUAL (42, static_cast<int>(n[foo_bar]));

  //object extreme numbers
  s = R"({ "min": -1.0e+28, "max": 1.0e+28 })";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (-1e28, static_cast<double>(n["min"]));
  CHECK_EQUAL (1e28, static_cast<double>(n["max"]));

  //object long strings
  s = R"({"x":[{"id": "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}], "id": "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"})";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (n["x"][0]["id"], static_cast<string>(n["id"]));

  //array object string unicode
  s = R"({"title":"\u041f\u043e\u043b\u0442\u043e\u0440\u0430 \u0417\u0435\u043c\u043b\u0435\u043a\u043e\u043f\u0430" })";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (u8"Полтора Землекопа", static_cast<string>(n["title"]));

  //object with newlines
  s = R"({
"a": "b"
})";
  CHECK_EQUAL (erc::success, n.read (s));

  //string 1 2 3 UTF8 sequences
  s = R"(["\u0060\u012a\u12AB"])";
  CHECK_EQUAL (erc::success, n.read (s));

  //string accepted surrogate pair U+1f639, U+1f48d
  s = R"(["\ud83d\ude39\ud83d\udc8d"])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (u8"😹💍", static_cast<string>(n[0]));

  //string allowed escapes
  s = R"(["\"\\\/\b\f\n\r\t"])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (8, static_cast<string>(n[0]).size ());

  //backslash and u-escaped 0 (this might be an error in input string)
  s = R"(["\\u0000"])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (6, static_cast<string>(n[0]).size ());

  //previous test but more meaningful
  s = R"(["\\\u0000"])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (2, static_cast<string>(n[0]).size ());

  //backslash double quotes
  s = R"(["\""])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (1, static_cast<string>(n[0]).size ());

  //string comments
  s = R"(["a/*b*/c/*d//e"])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL ("a/*b*/c/*d//e", static_cast<string>(n[0]));

  //string double escape a
  s = R"(["\\a"])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL ("\\a", static_cast<string>(n[0]));

  //string double escape n
  s = R"(["\\n"])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL ("\\n", static_cast<string>(n[0]));

  //string escaped control character
  s = R"(["\u0012"])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL ('\x12', static_cast<string>(n[0])[0]);

  //string escaped non-character
  s = R"(["\uffff"])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (0xffff, (unsigned int)utf8::rune (static_cast<string>(n[0]).cbegin ()));

  //string last surrogates 1 and 2
  s = R"(["\uDBFF\uDFFF"])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (0x10ffff, (unsigned int)utf8::rune (static_cast<string>(n[0]).cbegin ()));

  // string escaped newline
  s = R"(["new\u000aline"])";
  CHECK_EQUAL (erc::success, n.read (s));

  //string one byte UTF8
  s = R"(["\u002c"])";
  CHECK_EQUAL (erc::success, n.read (s));
  CHECK_EQUAL (",", (string)n[0]);

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

TEST (Code_project)
{
  string in = R"(
{
  "city_data":[
     {
       "t":"m",
       "l":[12.0,10.3,0.0,1.0]
     },
     {
       "t":"l",
       "l":[10.1,20.37,0.0,1.0]
     },
     {
       "t":"l",
       "l":[47.82,4.63,0.0,1.0]
     },
     {
       "t":"m",
       "l":[67.66,43.33,0.0,1.0]
     }
  ],
  "map_data":"JZDKZTCaTyWQymUwmk8lkMplMJpPJZDKZTCaTyWQymUwmk/8/+n8AVAZ1WCxk8rYAAAAASUVORK5CYII="
})";
  json::node data;
  data.read (in);

  json::node& cityInfo = data["city_data"];
  int i;
  for (i = 0; i < cityInfo.size (); i++)
  {
    json::node& cityType = cityInfo[i];
    std::string ctyTyp = (std::string)cityType["t"];
    json::node& cityLoc = cityType["l"];
    std::float_t x = (float)cityLoc[0];
    std::float_t y = (float)cityLoc[1];
    // do something with the retrieved values
    cityInfo[i]["newval"] = to_string (i + 1);
  }
  json::node new_node;
  new_node.read(R"({"t":"x", "l" : [0.0, 0.0, 0.0, 0.0] })");
  cityInfo[i] = new_node ;
  CHECK_EQUAL (5, data["city_data"].size ());
}

}  //end suite

#include <utpp/utpp.h>
#include <mlib/base64.h>

using namespace mlib;

//Test vectors from RFC4648
const char *expect[] = {
  "Zg==",
  "Zm8=",
  "Zm9v",
  "Zm9vYg==",
  "Zm9vYmE=",
  "Zm9vYmFy" };
SUITE (Base64)
{
TEST (Encode)
{
  const char *v = "foobar";

  for (int i = 1; i <= 6; i++)
  {
    char result[256];
    size_t len;
    len = base64enc (v, result, i);
    CHECK_EQUAL (len, strlen (expect[i - 1])+1);
    CHECK_EQUAL (expect[i - 1], result);
  }

  //test string-aware version
  for (int i = 1; i <= 6; i++)
  {
    std::string in ("foobar", i);
    std::string out = base64enc (in);
    std::string ok (expect[i - 1]);
    CHECK_EQUAL (ok, out);
  }
}

TEST (Encode_Zero_Length)
{
  char result[256];
  size_t len;
  len = base64enc ("", result, 0);

  CHECK_EQUAL (1, len);
  CHECK_EQUAL ("", result);

  //test string-aware version
  std::string in;
  std::string out = base64enc (in);
  CHECK_EQUAL (0, out.size());
  CHECK_EQUAL (in, out);
}

TEST (Decode)
{
  char out[256];
  size_t len;
  std::string outstr;
  const char *foobar = "foobar";

  memset (out, 0, sizeof (out));
  len = base64dec ("TWFu", out);  //from Wikipedia Base64 page
  CHECK_EQUAL (len, 3);
  CHECK_EQUAL ("Man", out);

  outstr = base64dec ("TWFu");
  CHECK_EQUAL ("Man", outstr);

  //Check test vectors from RFC4648
  for (int i = 0; i < 6; i++)
  {
    memset (out, 0, sizeof (out));
    len = base64dec (expect[i], out);
    CHECK_EQUAL (len, i+1);
    CHECK (!strncmp (out, foobar, len));

    outstr = base64dec (expect[i]);
    CHECK_EQUAL (std::string(foobar, i+1), outstr);
  }
}

} //end suite

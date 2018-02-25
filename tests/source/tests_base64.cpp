#include <utpp/utpp.h>
#include <mlib/base64.h>

using namespace mlib;

//Test vectors from RFC4648
TEST (Base64_Encode)
{
  const char *v = "foobar";
  const char *expect[] = { 
    "Zg==",
    "Zm8=",
    "Zm9v",
    "Zm9vYg==",
    "Zm9vYmE=",
    "Zm9vYmFy" };

  for (int i = 1; i <= 6; i++)
  {
    char result[256];
    size_t len;
    len = base64enc (v, result, i);
    CHECK_EQUAL (len, strlen (expect[i - 1])+1);
    CHECK_EQUAL (expect[i - 1], result);
  }
}

TEST (Base64_Encode_Zero_Length)
{
  char result[256];
  size_t len;
  len = base64enc ("", result, 0);

  CHECK_EQUAL (1, len);
  CHECK_EQUAL ("", result);
}

TEST (Base64_Decode)
{
  char out[256];
  size_t len;

  memset (out, 0, sizeof (out));
  len = base64dec ("TWFu", out);  //from Wikipedia Base64 page
  CHECK_EQUAL (len, 3);
  CHECK_EQUAL ("Man", out);

  memset (out, 0, sizeof (out));
  len = base64dec ("Zg==", out);
  CHECK_EQUAL (len, 1);
  CHECK_EQUAL ("f", out);

  memset (out, 0, sizeof (out));
  len = base64dec ("Zm8=", out);
  CHECK_EQUAL (len, 2);
  CHECK_EQUAL ("fo", out);

  memset (out, 0, sizeof (out));
  len = base64dec ("Zm9v", out);
  CHECK_EQUAL (len, 3);
  CHECK_EQUAL ("foo", out);

  memset (out, 0, sizeof (out));
  len = base64dec ("Zm9vYg==", out);
  CHECK_EQUAL (len, 4);
  CHECK_EQUAL ("foob", out);

  memset (out, 0, sizeof (out));
  len = base64dec ("Zm9vYmE=", out);
  CHECK_EQUAL (len, 5);
  CHECK_EQUAL ("fooba", out);

  memset (out, 0, sizeof (out));
  len = base64dec ("Zm9vYmFy", out);
  CHECK_EQUAL (len, 6);
  CHECK_EQUAL ("foobar", out);
}
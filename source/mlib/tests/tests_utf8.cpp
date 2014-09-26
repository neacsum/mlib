#include <windows.h>
#include <mlib/utf8.h>
#include <utpp/utpp.h>

using namespace std;

TEST (widen_string)
{
  string s1 ("ABCD");
  wstring l1(L"ABCD");

  wstring l2 = widen (s1);

  CHECK (l1 == l2);
}

TEST (widen_ptr)
{
  char *s1 = "ABCD";
  wstring l1(L"ABCD");

  wstring l2 = widen (s1);

  CHECK (l1 == l2);
}

TEST (narrow_string)
{
  wstring l1(L"ABCD");
  string s1 = narrow(l1);

  CHECK ("ABCD" == s1);
}

TEST (narrow_ptr)
{
  wchar_t *l1 = L"ABCD";
  string s1 = narrow(l1);

  CHECK ("ABCD" == s1);
}

TEST (widen_narrow)
{
  char *ptr = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  CHECK_EQUAL (ptr, narrow(widen(ptr)).c_str());
}


TEST (greek_letters)
{
  wchar_t *greek = L"ελληνικό αλφάβητο";
  string s = narrow (greek);
  CHECK (widen (s) == greek);
}

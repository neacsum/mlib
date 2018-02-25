﻿#include <windows.h>
#include <mlib/utf8.h>
#include <utpp/utpp.h>

using namespace std;
using namespace utf8;

SUITE (Utf8)
{
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

TEST (string_len)
{
  wchar_t *greek = L"ελληνικό αλφάβητο";
  string s = narrow (greek);
  size_t l = length (s);
  CHECK_EQUAL (wcslen (greek), l);
}

TEST (wemoji)
{
  wchar_t *wsmiley = L"😄";
  size_t wlen = wcslen (wsmiley);
  CHECK_EQUAL (2, wlen);
  string smiley = narrow (wsmiley);
  CHECK_EQUAL ("\xF0\x9f\x98\x84", smiley);
}

TEST (rune)
{
  string smiley{ u8"😀" };
  int rune_smiley = rune (smiley.begin ());
  CHECK_EQUAL (0x1f600, rune_smiley);
}

TEST (next)
{
  string emojis{ u8"😃😎😛" };
  int i = 0;
  auto ptr = emojis.begin ();
  while (ptr != emojis.end ())
  {
    i++;
    CHECK (next (emojis, ptr));
  }

  CHECK_EQUAL (3, i);
}

TEST (runes)
{
  string emojis{ u8"😃😎😛" };
  u32string emojis32 = runes (emojis);
  CHECK_EQUAL (3, emojis32.size ());
  CHECK_EQUAL (0x1f603, emojis32[0]);
}


TEST (dir)
{
  /* Make a folder using Greek alphabet, change current directory into it,
  obtain the current working directory and verify that it matches the name
  of the newly created folder */

  string dirname = u8"ελληνικό";
  CHECK (mkdir (dirname));   //mkdir returns true  for success

  //enter newly created directory
  CHECK (chdir (dirname));   //chdir returns true for success

  //Path returned by getcwd should end in our Greek string
  string cwd = getcwd ();
  size_t idx = cwd.rfind ("\\");      //last backslash
  string last = cwd.substr (idx+1);
  CHECK_EQUAL (dirname, last);

  //Move out of directory and remove it
  chdir ("..");
  CHECK (rmdir (dirname));    //rmdir returrs true for success
}

TEST (out_stream)
{
  /* Write some text in a file with a UTF8 encoded filename. Verifies using
  standard Windows file reading that content was written. */

  string filename = u8"ελληνικό";
  string filetext{ u8"😃😎😛" };

  utf8::ofstream u8strm(filename);

  u8strm << filetext << endl;
  u8strm.close ();

  HANDLE f = CreateFile (utf8::widen (filename).c_str (), GENERIC_READ, 0,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  CHECK (f);

  char read_back[80];
  memset (read_back, 0, sizeof (read_back));

  size_t len = filetext.size();
  DWORD nr;
  ReadFile (f, read_back, (DWORD)len, &nr, NULL);
  CloseHandle (f);
  CHECK (remove (filename));
  CHECK_EQUAL (len, nr);
  CHECK_EQUAL (filetext, read_back);
}

TEST (in_stream)
{
  /* write some stuff in file using utf8::ofstream object and read it
  back using utf8::ifstream. Verify read back matches original.*/

  string filetext{ u8"ελληνικό" };
  string filename{ u8"😃😎😛" };

  utf8::ofstream u8out (filename);

  u8out << filetext << endl;
  u8out.close ();

  utf8::ifstream u8in (filename);

  char read_back[80];
  memset (read_back, 0, sizeof (read_back));
  u8in.getline (read_back, sizeof (read_back));

  CHECK_EQUAL (filetext, read_back);

  u8in.close ();
  CHECK (remove (filename));
}
}
#include <mlib/defs.h>
#include <mlib/profile.h>
#include <mlib/utf8.h>
#include <utpp/utpp.h>

using namespace mlib;

/* 
  Write some keys and verify their retrieval.
*/
TEST (PutGetString)
{
  char val[80];
  _unlink ("test.ini");
  Sleep (200);
  Profile test ("test.ini");
  CHECK(test.PutString ("key0", "value00", "section0"));
  CHECK(test.PutString ("key1", "value01", "section0"));
  CHECK(test.PutString ("key0", "value10", "section1"));
  CHECK(test.PutString ("key1", "value11", "section1"));
  CHECK_EQUAL (strlen("value11"), test.GetString (val, sizeof(val), "key1", "section1", "default"));
  CHECK_EQUAL ("value11", val);
  _unlink ("test.ini");
}

/*
  Check PutString, PutDouble, PutInt, PutBool fail when ini file cannot be created
*/
TEST (IniNotCreated)
{
  Profile test ("inexistent folder\\test.ini");
  CHECK (!test.PutString ("key0", "value00", "section0"));
  CHECK (!test.PutDouble ("key1", 123.45, "section0"));
  CHECK (!test.PutInt ("key1", 123, "section0"));
  CHECK (!test.PutBool ("key1", true, "section0"));
}

/*
  Modify a key value verify it has changed.
*/
TEST (PutReplaceGet)
{
  char val[80];
  _unlink ("test.ini");
  Sleep (200);
  Profile test ("test.ini");
  CHECK(test.PutString ("key0", "value00", "section0"));
  CHECK(test.PutString ("key0", "newval", "section0"));
  int sz = test.GetString (val, sizeof(val), "key0", "section0");
  CHECK_EQUAL ("newval", val);
  CHECK_EQUAL (strlen("newval"), sz);
  _unlink ("test.ini");
}

/*
  Test key removal. Write 3 keys and delete the 2nd one. Verify that first and
  3rd are unchanged. Verify that GET for deleted key returns default value.
*/
TEST (PutDelete)
{
  char val[80];
  _unlink ("test.ini");
  Sleep (200);
  Profile test ("test.ini");
  test.PutString ("key0", "value00", "section0");
  test.PutString ("key1", "value01", "section0");
  test.PutString ("key2", "value02", "section0");
  test.DeleteKey ("key1", "section0");
  CHECK_EQUAL (1234, test.GetInt ("key1", "section0", 1234));
  test.GetString (val, sizeof(val), "key2", "section0");
  CHECK_EQUAL ("value02", val);
  test.GetString (val, sizeof(val), "key0", "section0");
  CHECK_EQUAL ("value00", val);
  _unlink ("test.ini");
}

/*
  Write 2 sections in an verify that GetSections retrieves them.
*/
TEST (GetSections)
{
  char sections[80];
  _unlink ("test.ini");
  Sleep (200);
  Profile test ("test.ini");
  test.PutString ("key0", "value00", "section0");
  test.PutString ("key1", "value01", "section0");
  test.PutString ("key0", "value10", "section1");
  test.PutString ("key1", "value11", "section1");

  test.GetSections (sections, sizeof(sections));
  CHECK_EQUAL ("section0", sections);
  CHECK_EQUAL ("section1", sections+strlen("section0")+1);
  _unlink ("test.ini");
}

TEST (GetSections_SmallBuffer)
{
  char sections[20];
  _unlink ("test.ini");
  Sleep (200);
  Profile test ("test.ini");
  test.PutString ("key0", "value00", "Asection12345678901234567890");
  test.PutString ("key1", "value01", "Bsection12345678901234567890");

  int ret = test.GetSections (sections, sizeof(sections));
  CHECK_EQUAL ("Asection1234567890", sections);
  CHECK_EQUAL (2, ret); //returns all sections not only the ones copied
  _unlink ("test.ini");

}

TEST (GetKeys)
{
  char buffer[256];
  _unlink ("test.ini");
  Sleep (200);
  Profile test ("test.ini");
  test.PutString ("key0", "value00", "section0");
  test.PutString ("key1", "value01", "section0");
  test.PutString ("key2", "value02", "section0");
  int i = test.GetKeys (buffer, sizeof(buffer), "section0");
  CHECK_EQUAL (3, i);
  char *p = buffer;
  CHECK_EQUAL ("key0", p);
  p += strlen(p)+1;
  CHECK_EQUAL ("key1", p);
  p += strlen(p)+1;
  CHECK_EQUAL ("key2", p);
  p += strlen(p)+1;
  CHECK_EQUAL ("", p);
  _unlink ("test.ini");
}

TEST (GetKeys_SmallBuffer)
{
  char buffer[20];
  _unlink ("test.ini");
  Sleep (200);
  Profile test("test.ini");
  test.PutString ("key0", "value00", "section0");
  test.PutString ("key1", "value01", "section0");
  test.PutString ("key2_01234567890123456789", "value02", "section0");
  int i = test.GetKeys (buffer, sizeof(buffer), "section0");
  CHECK_EQUAL (3, i);
  char *p = buffer;
  CHECK_EQUAL ("key0", p);
  p += strlen(p)+1;
  CHECK_EQUAL ("key1", p);
  p += strlen(p)+1;
  CHECK_EQUAL ("key2_012", p);
  p += strlen(p)+1;
  CHECK_EQUAL ("", p);
  _unlink ("test.ini");
}

TEST (HasSection)
{
  _unlink ("test.ini");
  Sleep (200);
  Profile test ("test.ini");
  test.PutString ("key0", "value00", "section0");
  test.PutString ("key1", "value01", "section0");
  bool t = test.HasSection ("section0");
  CHECK_EQUAL (true, t);
  t= test.HasSection ("no_section");
  CHECK_EQUAL (false, t);
  _unlink ("test.ini");
}

/*
  Write a non-ANSI value in an INI file with non-ANSI name.
*/
TEST (Greek_filename)
{
  char *filename = "\xCE\xB1\xCE\xBB\xCF\x86\xCE\xAC\xCE\xB2\xCE\xB7\xCF\x84\xCE\xBF.ini";
  char *greek_text = "\xCE\xB1\xCE\xBB\xCF\x86\xCE\xAC\xCE\xB2\xCE\xB7\xCF\x84\xCE\xBF";
  char strval[80];

  _wunlink (widen(filename).c_str());
  Sleep (200);
  Profile greek (filename);
  greek.PutInt ("Integer", 1, "Keys");
  CHECK_EQUAL (1, greek.GetInt ("Integer", "Keys", 2));
  greek.PutString ("GreekAlphabet", greek_text, "Keys");
  greek.GetString (strval, sizeof(strval), "GreekAlphabet", "Keys");
  CHECK_EQUAL (strval, greek_text);
  greek.PutString (greek_text, "This is how you spell alphabet in Greek", "Keys");
  _wunlink (widen(filename).c_str());
}

TEST (Quoted_strings)
{
  char *quoted = "\"Quoted String with \" in the middle\"";
  char buffer[256];
  _unlink ("test.ini");
  Sleep (200);
  Profile test ("test.ini");
  test.PutString ("key0", quoted, "section");
  test.GetString (buffer, sizeof(buffer), "key0", "section");
  CHECK_EQUAL (quoted, buffer);
  _unlink ("test.ini");  
}

TEST (Copy_section)
{
  char buffer[256];

  Profile f1 ("test1.ini");
  f1.PutString ("key0", "value00", "section0");
  f1.PutString ("key1", "value01", "section0");

  Profile f2 ("test2.ini");
  f2.CopySection (f1, "section0", "section1");
  f2.GetString (buffer, sizeof(buffer), "key0", "section1");
  CHECK_EQUAL ("value00", buffer);
  f2.GetString (buffer, sizeof(buffer), "key1", "section1");
  CHECK_EQUAL ("value01", buffer);
  _unlink ("test1.ini");
  _unlink ("test2.ini");
}
#include <utpp/utpp.h>
#include <mlib/rdir.h>
#include <utf8/utf8.h>
#include <mlib/basename.h>

using namespace mlib;

SUITE (rdir)
{
  TEST (rdir1)
  {
    int ret;

    //create a long path
    ret = r_mkdir ("aa\\bbb\\cccc");
    CHECK_EQUAL (0, ret);

    //verify we can access all components
    CHECK (utf8::access ("aa", 0));
    CHECK (utf8::access ("aa\\bbb", 0));
    CHECK (utf8::access ("aa\\bbb\\cccc", 0));

    //erase it
    ret = r_rmdir ("aa\\bbb\\cccc");
    CHECK_EQUAL (0, ret);

    //verify it's all gone
    CHECK (!utf8::access ("aa", 0));
  }

  //same as rdir1 for absolute paths
  TEST (rdir_absolute)
  {
    int ret;

    ret = r_mkdir ("C:\\aa\\bbb\\cccc");
    CHECK_EQUAL (0, ret);

    CHECK (utf8::access ("c:\\aa", 0));
    CHECK (utf8::access ("c:\\aa\\bbb", 0));
    CHECK (utf8::access ("c:\\aa\\bbb\\cccc", 0));

    ret = r_rmdir ("c:\\aa\\bbb\\cccc");
    CHECK_EQUAL (0, ret);

    CHECK (!utf8::access ("c:\\aa", 0));
  }

  //relative paths starting with current folder
  TEST (rdir_dot)
  {
    int ret;

    ret = r_mkdir (".\\aa\\bbb\\cccc");
    CHECK_EQUAL (0, ret);

    CHECK (utf8::access (".\\aa", 0));
    CHECK (utf8::access (".\\aa\\bbb", 0));
    CHECK (utf8::access (".\\aa\\bbb\\cccc", 0));

    ret = r_rmdir (".\\aa\\bbb\\cccc");
    CHECK_EQUAL (0, ret);

    CHECK (!utf8::access (".\\aa", 0));
  }

  //relative paths starting with parent folder
  TEST (rdir_dotdot)
  {
    int ret;

    ret = r_mkdir ("..\\aa\\bbb\\cccc");
    CHECK_EQUAL (0, ret);

    CHECK (utf8::access ("..\\aa", 0));
    CHECK (utf8::access ("..\\aa\\bbb", 0));
    CHECK (utf8::access ("..\\aa\\bbb\\cccc", 0));

    ret = r_rmdir ("..\\aa\\bbb\\cccc");
    CHECK_EQUAL (0, ret);

    CHECK (!utf8::access ("..\\aa", 0));
  }

  //cannot remove not empty folder
  TEST (rdir_file)
  {
    const char* s = "c:\\aa\\bbb\\cccc\\file.txt";
    int ret;

    //Look how simple is to open a file in a deep hierarchy
    ret = r_mkdir (dirname (s));
    FILE *f = fopen (s, "w");

    CHECK_EQUAL (0, ret);
    CHECK (f);
    fputs ("abcd", f);
    fclose (f);

    //r_rmdir fails on not empty directory
    ret = r_rmdir ("c:\\aa\\bbb\\cccc");
    CHECK_EQUAL (ENOTEMPTY, ret);

    //... but succeeds when directory is empty
    utf8::remove (s);
    ret = r_rmdir ("c:\\aa\\bbb\\cccc");
    CHECK_EQUAL (0, ret);
  }

  // All functions work also with forward slash instead of back slash
  TEST (rdir_fwd_slash)
  {
    const char* s = "c:/aa/bbb/cccc/file.txt";
    int ret;

    //Look how simple is to open a file in a deep hierarchy
    ret = r_mkdir (dirname (s));
    FILE *f = fopen (s, "w");

    CHECK_EQUAL (0, ret);
    CHECK (f);
    fputs ("abcd", f);
    fclose (f);

    //r_rmdir fails on not empty directory
    ret = r_rmdir ("c:/aa/bbb/cccc");
    CHECK_EQUAL (ENOTEMPTY, ret);

    //... but succeeds when directory is empty
    utf8::remove (s);
    ret = r_rmdir ("c:/aa/bbb/cccc");
    CHECK_EQUAL (0, ret);
  }
}

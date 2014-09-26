#include <windows.h>
#include <mlib/basename.h>
#include <utpp/utpp.h>


TEST (dirname)
{
  //simple tests with regular filenames
  CHECK_EQUAL ("c:\\path1\\path2", dirname ("c:\\path1\\path2\\file.ext"));
  CHECK_EQUAL ("c:\\path1\\path2", dirname ("c:\\path1\\path2\\file"));
  CHECK_EQUAL ("c:\\path1\\path2", dirname ("c:\\path1\\path2\\"));
}

TEST (dirname_special)
{
  //Special cases: null argument, empty name, etc.
  CHECK_EQUAL (".", dirname (NULL));
  CHECK_EQUAL (".", dirname (""));
  CHECK_EQUAL (".", dirname ("file.ext"));
  CHECK_EQUAL ("C:", dirname ("C:\\root.ext"));
}

TEST (dirname_unc)
{
  //UNC and HTML-like filenames
  CHECK_EQUAL ("\\\\server\\share$\\path1\\path2", 
      dirname ("\\\\server\\share$\\path1\\path2\\file.ext"));
  CHECK_EQUAL ("file://///servername/share",
      dirname ("file://///servername/share/file.txt"));
}

TEST (basename)
{
  //simple tests with regular filenames
  CHECK_EQUAL ("file.ext", basename ("c:\\path1\\path2\\file.ext"));
  CHECK_EQUAL ("file", basename ("c:\\path1\\path2\\file"));
  CHECK_EQUAL (".", basename ("c:\\path1\\path2\\"));
}

TEST (basename_special)
{
  //Special cases: null argument, empty name, etc.
  CHECK_EQUAL (".", basename (NULL));
  CHECK_EQUAL (".", basename (""));
  CHECK_EQUAL ("file.ext", basename ("file.ext"));
}

TEST (basename_unc)
{
  //UNC and HTML-like filenames
  CHECK_EQUAL ("file.ext", 
     basename ("\\\\server\\share$\\path1\\path2\\file.ext"));
  CHECK_EQUAL ("file.txt",
     basename ("file://///servername/share/file.txt"));
  CHECK_EQUAL (".",
     basename ("file://///servername/share/"));
}

TEST (basename_utf8)
{
  //UTF-8 encoded filenames
  CHECK_EQUAL ("αλφάβητο.ini", basename ("c:\\αλφάβητο.ini"));
  CHECK_EQUAL ("哈利法克斯.αλφ", basename ("c:\\path\\哈利法克斯.αλφ"));
}

TEST (dirname_utf8)
{
  //UTF-8 encoded filenames
  CHECK_EQUAL ("α:\\αλφάβητο", dirname("α:\\αλφάβητο\\file.ext"));
}
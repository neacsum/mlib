/*
  ERROR_CODES.CPP - Demo program for erc and checked<> classes
  (c) Mircea Neacsu 2024. Licensed under MIT License.
  See README file for full license terms.
  (https://github.com/neacsum/mlib#readme)
*/

#define _CRT_SECURE_NO_WARNINGS

#include <mlib/errorcode.h>
#include <cstdio>
#include <iostream>
#include <errno.h>


using mlib::checked, mlib::erc;


checked<FILE*> my_open (const std::string& fname)
{
  checked<FILE*> ret;
  FILE* f = fopen (fname.c_str (), "r");
  if (f)
    ret = f;
  else
    ret = erc (errno);

  return ret;
}

// An error facility class that uses strerror function for formatting
class facility_str : public mlib::errfac
{
public:
  facility_str () {}
  std::string message (const erc& e) const override
  {
    return strerror (e);
  }
};


// A function that uses checked<> with a try block
erc checked_and_try (const std::string& filename)
{
  try
  {
    // fp is a checked<FILE*> object
    auto fp = my_open (filename);
    char buffer[256];

    // In the next line note the additional indirection on fp.
    // This is needed because the FILE* is wrapped in a checked<> envelope
    auto n = fread (buffer, sizeof (char), sizeof (buffer), *fp);

    std::cout << "Successful read from %s" << filename << std::endl;
    fclose (*fp);
  }
  catch (erc& x)
  {
    return x;
  }
  return erc::success;
}

int main (int argc, char** argv)
{
  std::cout << "Opening an existing file...";
  checked_and_try (argv[0]);

  /* Try to open an inexistent file.Using default error facility produces
     an error message that has only the error number (errno). */
  std::cout << "Open inexistent file...";
  if (erc::success == checked_and_try ("this file does not exist"))
    std::cout << "success... surprise!!" << std::endl;

  /* Replace default facility with one that formats the error message.
     You could have language dependent messages */
  facility_str f;
  mlib::errfac::Default (&f);

  // Do the same thing again but now we should get a nicer error message
  std::cout << "Open inexistent file...";
  auto fp = my_open ("this file does not exist");
  if (fp == mlib::erc::success)
    std::cout << "success... surprise!!" << std::endl;
  else
    std::cout << fp.message () << std::endl;

  // Let checked<> objects throw an exception
  try
  {
    fp = checked_and_try ("this file does not exist");
    char buffer[256];
    fread (buffer, sizeof (char), sizeof (buffer), *fp);
    fclose (*fp);
  }
  catch (erc& x)
  {
    std::cout << "Exception - " << x.message () << std::endl;
  }


  return 0;
}
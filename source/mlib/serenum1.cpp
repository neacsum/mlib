/*!
  \file serenum1.cpp - Implementation of SerEnum_UsingCreateFile() function.
  (c) Mircea Neacsu 2017. All rights reserved.

  These functions are heavily inspired from [CEnumerateSerial] (http://www.naughter.com/enumser.html)
  code.
*/
#include <mlib/serenum.h>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif


/*!
  \addtogroup serenum
  Iterate from 1 to 255 finding those ports for which the CreateFile function
  doesn't fail.
*/
bool SerEnum_UsingCreateFile (std::vector<int>& ports)
{
  ports.clear ();

  /* Up to 255 COM ports are supported so we iterate through all of them seeing
  if we can open them or if we fail to open them, get an access denied or general error.
  Both of these cases indicate that there is a COM port at that number. */
  for (int i = 1; i<256; i++)
  {
    //Form the raw device name
    wchar_t port_str[32];
    swprintf (port_str, _countof(port_str), L"\\\\.\\COM%d", i);
    bool ok = false;

    //Try to open the port
    HANDLE h = CreateFile (port_str, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if (h == INVALID_HANDLE_VALUE)
    {
      DWORD error = GetLastError ();

      //Check to see if the error was because some other app had the port open or a general failure
      if (error == ERROR_ACCESS_DENIED || error == ERROR_GEN_FAILURE || error == ERROR_SHARING_VIOLATION || error == ERROR_SEM_TIMEOUT)
        ok = true;
    }
    else
      ok = true;

    //Add the port number to the array which will be returned
    if (ok)
      ports.push_back (i);
  }
  return true;
}

#ifdef MLIBSPACE
};
#endif

/*!
  \file serenum2.cpp - Implementation of SerEnum_UsingRegistry() function.
  (c) Mircea Neacsu 2017. All rights reserved.

  These functions are heavily inspired from [CEnumerateSerial] (http://www.naughter.com/enumser.html)
  code.
*/
#include <Windows.h>
#include <mlib/serenum.h>

using namespace std;

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/*!
  \addtogroup serenum
  Enumerates all values under HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM
  to retrieve available COM ports.
*/
bool SerEnum_UsingRegistry (vector<int>& ports)
{
  HKEY comm_key;
  if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, L"HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ , &comm_key) != ERROR_SUCCESS)
    return false;


  //Get the max value name and max value lengths
  DWORD max_value_name_len = 0,
    max_value_len = 0,
    nvalues = 0;
  if (RegQueryInfoKey (comm_key, nullptr, nullptr, nullptr, nullptr, nullptr, 
    nullptr, &nvalues, &max_value_name_len, &max_value_len, nullptr, nullptr) != ERROR_SUCCESS)
  {
    RegCloseKey (comm_key);
    return false;
  }

  //Allocate some space for the value and value name
  wstring value_name(max_value_name_len+1, L'\0');
  wstring value (max_value_len+1, L'\0');

  //Enumerate all the values underneath HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM
  bool continue_enumeration = true;
  DWORD index = 0;
  while (continue_enumeration)
  {
    max_value_name_len = (DWORD)value_name.length ();
    max_value_len = (DWORD)(DWORD)value.length ();
    DWORD type;
    int port_num;
    DWORD ret = RegEnumValue (comm_key, index, &value_name[0], &max_value_name_len, nullptr, &type, (BYTE*)&value[0], &max_value_len);
    continue_enumeration = (ret == ERROR_SUCCESS);
    if (continue_enumeration)
    {
      if ((type == REG_SZ) && swscanf (value.c_str (), L"COM%d", &port_num) == 1)
        ports.push_back (port_num);

      //Prepare for the next loop
      index++;
    }
  }
  return true;
}

#ifdef MLIBSPACE
};
#endif

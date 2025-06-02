/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

#include <mlib/mlib.h>
#pragma hdrstop
#include <utf8/utf8.h>


namespace mlib {

/*!
  \ingroup serenum
  Enumerates all values under `HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM`
  to retrieve available COM ports.
*/
bool SerEnum_UsingRegistry (std::vector<int>& ports)
{
  HKEY comm_key;
  if (utf8::RegOpenKey (HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", comm_key, KEY_READ)
      != ERROR_SUCCESS)
    return false;

  std::vector<std::string> values;
  if (utf8::RegEnumValue (comm_key, values) != ERROR_SUCCESS)
    return false;

  for (auto& v : values)
  {
    int p;
    std::string port;
    utf8::RegGetValue (comm_key, "", v, port);
    if (sscanf (port.c_str (), "COM%d", &p) == 1)
      ports.push_back (p);
  }
  return true;
}

} // namespace mlib

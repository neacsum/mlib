/*!
  \file serenum2.cpp - Implementation of SerEnum_UsingSetupAPI() function.
  (c) Mircea Neacsu 2017. All rights reserved.

  These functions are heavily inspired from [CEnumerateSerial] (http://www.naughter.com/enumser.html)
  code.
*/
#include <Windows.h>
#include <mlib/serenum.h>
#include <mlib/utf8.h>

#include <SetupAPI.h>
#pragma comment(lib, "setupapi.lib")

using namespace std;

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/*!
  \addtogroup serenum
  Uses SetupDiEnumDeviceInfo function to retrieve available COM ports and their
  friendly name.
*/
bool SerEnum_UsingSetupAPI (std::vector<int>& ports, std::vector<std::string>& names)
{
  ports.clear ();
  names.clear ();

  //Create a "device information set" for the specified GUID
  HDEVINFO hDevInfoSet = SetupDiGetClassDevs (&GUID_DEVINTERFACE_COMPORT, 
    nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (hDevInfoSet == INVALID_HANDLE_VALUE)
    return false;

  //Finally do the enumeration
  bool more_items = true;
  int nIndex = 0;
  SP_DEVINFO_DATA devInfo;
  devInfo.cbSize = sizeof (SP_DEVINFO_DATA);
  while (more_items)
  {
    //Enumerate the current device
    more_items = SetupDiEnumDeviceInfo (hDevInfoSet, nIndex, &devInfo);
    if (more_items)
    {
      //Get the registry key which stores the ports settings
      HKEY device_key;
      device_key = SetupDiOpenDevRegKey (hDevInfoSet, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
      if (device_key != INVALID_HANDLE_VALUE)
      {
        int port;
        DWORD type;
        wchar_t port_name[256];
        DWORD size = _countof (port_name);
        if (RegQueryValueEx (device_key, L"PortName", nullptr, &type, (BYTE*)port_name, &size) == ERROR_SUCCESS
          && (type == REG_SZ || type == REG_EXPAND_SZ)
          && (swscanf (port_name, L"COM%d", &port) == 1))
        {
          ports.push_back (port);

          //get friendly name now
          SetupDiGetDeviceRegistryProperty (hDevInfoSet, &devInfo, SPDRP_FRIENDLYNAME, &type, nullptr, 0, &size);
          wstring friendly (size, L'\0');
          if (SetupDiGetDeviceRegistryProperty (hDevInfoSet, &devInfo, SPDRP_FRIENDLYNAME, &type, (BYTE*)&friendly[0], size, &size)
           && (type == REG_SZ))
            names.push_back(utf8::narrow (friendly));
          else
            names.push_back ("");
        }
      }
    }

    ++nIndex;
  }

  //Free up the "device information set" now that we are finished with it
  SetupDiDestroyDeviceInfoList (hDevInfoSet);

  return true;
}

#ifdef MLIBSPACE
};
#endif

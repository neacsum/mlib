#include <mlib/serenum.h>
#include <mlib/utf8.h>

#include <SetupAPI.h>
#pragma comment(lib, "setupapi.lib")

using namespace std;

static bool RegQueryValueString (HKEY key, const std::string& value_name, std::string& value)
{
  wstring wname = utf8::widen (value_name);

  //First query for the size of the registry value 
  DWORD nchars = 0;
  if (RegQueryValueEx (key, wname.c_str (), nullptr, nullptr, nullptr, &nchars) != ERROR_SUCCESS)
    return false;

  //Allocate enough bytes for the return value
  DWORD allocated_size = ((nchars + 1) * sizeof (wchar_t)); //+1 is to allow us to null terminate the data if required
  wstring wvalue (allocated_size, L'\0');

  DWORD dwType = 0;
  ULONG nBytes = allocated_size;
  if (RegQueryValueEx (key, wname.c_str (), nullptr, &dwType, (BYTE*)(&wvalue[0]), &nBytes) != ERROR_SUCCESS)
    return false;

  if ((dwType != REG_SZ) && (dwType != REG_EXPAND_SZ) )
  {
    SetLastError (ERROR_INVALID_DATA);
    return false;
  }
  value = utf8::narrow (wvalue);
  return true;
}

BOOL QueryRegistryPortName (HKEY device_key, int& port)
{
  //What will be the return value from the method (assume the worst)
  BOOL ret = FALSE;

  //Read in the name of the port
  string port_name;
  if (RegQueryValueString (device_key, "PortName", port_name))
  {
    //Check if it looks like "COMx"
    if (port_name.length () > 3 && sscanf (port_name.c_str(), "COM%d", &port) == 1)
      ret = true;
  }

  return ret;
}

static bool QueryFriendlyName (HDEVINFO hDevInfoSet, SP_DEVINFO_DATA& devInfo, std::string& name)
{
  DWORD dwType = 0;
  DWORD dwSize = 0;
  //Query initially to get the buffer size required
  if (!SetupDiGetDeviceRegistryProperty (hDevInfoSet, &devInfo, SPDRP_FRIENDLYNAME, &dwType, nullptr, 0, &dwSize))
  {
    if (GetLastError () != ERROR_INSUFFICIENT_BUFFER)
      return false;
  }
  wstring friendly (dwSize, L'\0');
  BOOL ret = SetupDiGetDeviceRegistryProperty (hDevInfoSet, &devInfo, SPDRP_FRIENDLYNAME, &dwType, (BYTE*)&friendly[0], dwSize, &dwSize)
             && (dwType == REG_SZ);
  if (ret)
    name = utf8::narrow (friendly);
  return ret;
}


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
      //Did we find a serial port for this device
      BOOL bAdded = FALSE;

      //Get the registry key which stores the ports settings
      HKEY device_key;
      device_key = SetupDiOpenDevRegKey (hDevInfoSet, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
      if (device_key != INVALID_HANDLE_VALUE)
      {
        int nPort = 0;
        if (QueryRegistryPortName (device_key, nPort))
        {
          ports.push_back (nPort);
          bAdded = TRUE;
        }
      }

      //If the port was a serial port, then also try to get its friendly name
      if (bAdded)
      {
        std::string friendly_name;
        if (QueryFriendlyName (hDevInfoSet, devInfo, friendly_name))
          names.push_back (friendly_name);
        else
          names.push_back ("");
      }
    }

    ++nIndex;
  }

  //Free up the "device information set" now that we are finished with it
  SetupDiDestroyDeviceInfoList (hDevInfoSet);

  //Return the success indicator
  return true;
}


/*!
  \file firewall.cpp Implementation of firwall object

    (c) Mircea Neacsu 2007-2019
*/

#include <mlib/firewall.h>
#include <mlib/trace.h>
//#include <comutil.h>
//#include <comip.h>


namespace mlib {

BSTR ConvertStringToBSTR(const char* in);

errfac fw_errors("Firewall error");

#define FWERC(A) erc((A),erc::error, &fw_errors)
#define FWERROR(A) FWERC(A).raise()


firewall::firewall(void) :
  fwmgr(0), fwpolicy(0), fwprofile(0)
{
  HRESULT hr = S_OK;

  // Initialize COM.
  hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);

  // Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
  // initialized with a different mode. Since we don't care what the mode is,
  // we'll just use the existing mode.
  if (hr == RPC_E_CHANGED_MODE)
    hr = S_OK;

  if(FAILED(hr))
  {
    TRACE ("firewall::firewall - COInitializeEx err %x", hr);
    FWERROR (hr);
  }

  // Create an instance of the firewall settings manager.
  hr = CoCreateInstance(
    __uuidof(NetFwMgr),
    NULL,
    CLSCTX_INPROC_SERVER,
    __uuidof(INetFwMgr),
    (void**)&fwmgr
    );

  if(FAILED(hr))
  {
    TRACE ("firewall::firewall - CoCreateInstance(NetFwMgr) err %d", -hr);
    return;
  }

  // Retrieve the local firewall policy.
  hr = fwmgr->get_LocalPolicy(&fwpolicy);

  if(FAILED(hr))
  {
    TRACE ("firewall::firewall - get_LocalPolicy err %d", -hr);
    fwmgr->Release ();
    fwmgr = NULL;
    FWERROR (hr);
  }

  // Retrieve the firewall profile currently in effect.
  hr = fwpolicy->get_CurrentProfile(&fwprofile);

  if(FAILED(hr))
  {
    TRACE ("firewall::firewall - get_CurrentPorfile err %d", -hr);
    fwpolicy->Release ();
    fwpolicy = 0;
    fwmgr->Release ();
    fwmgr = 0;
    FWERROR (hr);
  }
}

firewall::~firewall(void)
{
  // Release the firewall profile.
  if (fwprofile != NULL)
    fwprofile->Release();

  if(fwpolicy != NULL)
    fwpolicy->Release();

  if(fwmgr != NULL)
    fwmgr->Release();

  CoUninitialize();
}

bool firewall::is_enabled ()
{
  HRESULT hr = S_OK;
  VARIANT_BOOL enabled = VARIANT_FALSE;
  TRACE ("firewall::is_enabled");

  // Get the current state of the firewall.
  TRACE ("fwprofile=%x", fwprofile);
  if (fwprofile)
  {
    hr = fwprofile->get_FirewallEnabled(&enabled);
    if(FAILED(hr))
    {
      TRACE ("firewall::is_enabled - get_FirewallEnabled err %d", -hr);
      FWERROR (hr);
    }
    TRACE ("firewall::is_enabled done (%d)", enabled);
  }
  return (enabled != VARIANT_FALSE);
}

bool firewall::is_port_enabled (int portnum, bool tcp)
{
  HRESULT hr = S_OK;
  NET_FW_IP_PROTOCOL protocol = tcp?NET_FW_IP_PROTOCOL_TCP : NET_FW_IP_PROTOCOL_UDP;
  VARIANT_BOOL enabled = VARIANT_FALSE;
  INetFwOpenPort* port = NULL;
  INetFwOpenPorts* allports = NULL;

  if (!fwprofile)
    return true;

  // Retrieve the globally open ports collection.
  hr = fwprofile->get_GloballyOpenPorts(&allports);

  if(FAILED(hr))
    FWERROR (hr);

  // Attempt to retrieve the globally open port.
  hr = allports->Item(portnum, protocol, &port);
  if (SUCCEEDED(hr))
  {
    // Find out if the globally open port is enabled.
    hr = port->get_Enabled(&enabled);

    if (FAILED(hr))
    {
      port->Release ();
      allports->Release ();
      FWERROR(hr);
    }

    port->Release();
  }

  allports->Release();
  return (enabled != VARIANT_FALSE);
}

bool firewall::has_port(int portnum, bool tcp)
{
  HRESULT hr;
  NET_FW_IP_PROTOCOL protocol = tcp?NET_FW_IP_PROTOCOL_TCP : NET_FW_IP_PROTOCOL_UDP;
  bool result = true;
  INetFwOpenPort* port = NULL;
  INetFwOpenPorts* allports = NULL;

  if (!fwprofile)
    return true;

  // Retrieve the globally open ports collection.
  hr = fwprofile->get_GloballyOpenPorts(&allports);

  if(FAILED(hr))
    FWERROR (hr);

  // Attempt to retrieve the globally open port.
  hr = allports->Item(portnum, protocol, &port);
  if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
  {
    result = false;
  }
  else if (FAILED(hr))
  {
    allports->Release ();
    FWERROR (hr);
  }
  else
    port->Release ();
  allports->Release ();
  return result;
}

bool firewall::has_app(const char *appname)
{
  HRESULT hr;
  bool result = true;
  INetFwAuthorizedApplication* app = NULL;
  INetFwAuthorizedApplications* allapps = NULL;

  if (!fwprofile)
    return true;

  // Retrieve the authorized applications collection.
  hr = fwprofile->get_AuthorizedApplications(&allapps);

  if(FAILED(hr))
    FWERROR (hr);

  // Attempt to retrieve the application.
  BSTR bname = ConvertStringToBSTR(appname);
  hr = allapps->Item(bname, &app);
  if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
    result = false;
  else if (FAILED(hr))
  {
    allapps->Release ();
    ::SysFreeString(bname);
    FWERROR (hr);
  }
  else
    app->Release ();
  allapps->Release ();
  ::SysFreeString(bname);
  return result;
}


erc firewall::add_port(int portnum, bool tcp, const char *name)
{
  HRESULT hr;
  NET_FW_IP_PROTOCOL protocol = tcp?NET_FW_IP_PROTOCOL_TCP : NET_FW_IP_PROTOCOL_UDP;
  INetFwOpenPort* port = NULL;
  INetFwOpenPorts* allports = NULL;

  if (!fwprofile)
    return erc::success;

  // Retrieve the globally open ports collection.
  hr = fwprofile->get_GloballyOpenPorts(&allports);

  if(FAILED(hr))
    FWERROR (hr);

  //create a new open port
  hr = CoCreateInstance( 
    __uuidof(NetFwOpenPort), 
    NULL, 
    CLSCTX_INPROC_SERVER, 
    __uuidof(INetFwOpenPort), 
    (void**)&port);

  if (FAILED(hr))
  {
    allports->Release ();
    FWERROR(hr);
  }

  //set port name
  BSTR bname = ConvertStringToBSTR(name);

  port->put_Name (bname);
  port->put_Port (portnum);
  port->put_Protocol (protocol);

  hr = allports->Add (port);
  port->Release ();
  allports->Release ();
  ::SysFreeString(bname);
  if (FAILED(hr))
    return FWERC(hr);

  return erc::success;
}

erc firewall::add_app(const char *appname, const char *filename)
{
  HRESULT hr;
  INetFwAuthorizedApplication* app = NULL;
  INetFwAuthorizedApplications* allapps = NULL;

  if (!fwprofile)
    return erc::success;

  // Retrieve the authorized applications collection.
  hr = fwprofile->get_AuthorizedApplications(&allapps);


  if(FAILED(hr))
    FWERROR (hr);

  //create a new open port
  hr = CoCreateInstance( 
    __uuidof(NetFwAuthorizedApplication), 
    NULL, 
    CLSCTX_INPROC_SERVER, 
    __uuidof(INetFwAuthorizedApplication), 
    (void**)&app);

  if (FAILED(hr))
  {
    allapps->Release ();
    FWERROR(hr);
  }

  //set app name and image name
  BSTR bapp = ConvertStringToBSTR(appname);
  BSTR bfile = ConvertStringToBSTR(filename);
  app->put_Name (bapp);
  app->put_ProcessImageFileName (bfile);

  hr = allapps->Add (app);
  app->Release ();
  allapps->Release ();

  ::SysFreeString(bapp);
  ::SysFreeString(bfile);

  if (FAILED(hr))
    return FWERC(hr);

  return erc::success;
}

erc firewall::set_port (int portnum, bool tcp, bool enable)
{
  HRESULT hr = S_OK;
  NET_FW_IP_PROTOCOL protocol = tcp?NET_FW_IP_PROTOCOL_TCP : NET_FW_IP_PROTOCOL_UDP;
  VARIANT_BOOL enabled = VARIANT_FALSE;
  INetFwOpenPort* port = NULL;
  INetFwOpenPorts* allports = NULL;

  if (!fwprofile)
    return erc::success;

  // Retrieve the globally open ports collection.
  hr = fwprofile->get_GloballyOpenPorts(&allports);

  if(FAILED(hr))
    FWERROR (hr);

  // Attempt to retrieve the globally open port.
  hr = allports->Item(portnum, protocol, &port);
  if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
  {
    hr = CoCreateInstance( 
          __uuidof(NetFwOpenPort), 
          NULL, 
          CLSCTX_INPROC_SERVER, 
          __uuidof(INetFwOpenPort), 
          (void**)&port);
    if (FAILED(hr))
      return FWERC(hr);
    hr = allports->Add (port);
    if (FAILED(hr))
      return FWERC(hr);
  }
  else if (FAILED(hr))
    return FWERC(hr);
  
  hr = port->put_Enabled(enabled?VARIANT_TRUE:VARIANT_FALSE);
  if (FAILED(hr))
      return FWERC(hr);
  port->Release();
  allports->Release();

  return erc::success;
}

BSTR ConvertStringToBSTR(const char* in)
{
  int cnt;
  BSTR out = NULL;

  if (!in)
    return NULL;

  if (cnt = MultiByteToWideChar (CP_UTF8, 0, in, -1, NULL, 0))//get size...
  {
    cnt--; //...minus NULL terminator
    if (out = SysAllocStringLen (NULL, cnt))
    {
      if (!MultiByteToWideChar (CP_UTF8, 0, in, -1, out, cnt))
      {
        if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
          return out;
        SysFreeString (out); //clean up
        out = NULL;
      }
    }
  }
  return out;
}

}

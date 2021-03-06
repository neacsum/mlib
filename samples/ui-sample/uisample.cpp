/*!
  UISAMPLE.CPP - This program demonstrates how to build a HTML user interface
  using the HTTP server and the JSONBridge interface.

  The program starts a HTTP server on port 8080 and opens a browser window to
  "localhost:8080" address. After that it continues living as a small icon in
  the systray. To end the program right click on the systray icon and select
  "Exit".

  The MIT License (MIT)

  (c) Mircea Neacsu 2017-2020

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <mlib/trace.h>
#include <mlib/wsockstream.h>
#include <mlib/critsect.h>
#include <mlib/thread.h>
#include <utf8/utf8.h>
#include <utf8/winutf8.h>
#include <mlib/basename.h>
#include <mlib/rdir.h>
#include <assert.h>
#include <mlib/log.h>
#include <direct.h>
//#include <nmea.h>
#include <mlib/convert.h>

#include "resource.h"
#include <mlib/jbridge.h>

using namespace std;
using namespace mlib;

#define SERVER_PORT   8080           //http port
#define SERVER_WNDCLASSNAME   L"uisample"
#define WM_TRAYNOTIFY     (WM_USER+1)

//globals
HINSTANCE           hInst;
string              docroot;
NOTIFYICONDATA      nid;
HWND                mainWnd;

//variables that are accessible through the user interface
char str1[256] {"A string of up to 256 chars"};
short hvar = -123;
unsigned short huvar = 0xffff;
int ivar = 123;
unsigned int iuvar = 0x80000000;
long lvar = 1234567;
unsigned long luvar = 0xffffffff;
float fvar = 123.45f;
double dvar = 123.45;
char *pstr = str1;
char str[80] {"Another string that is 80 chars long"};
bool bvar;
int iarr[4] {111, 222, 333, 444};
char sarr[4][80] {"THE", "THE QUICK", "THE QUICK BROWN", "THE QUICK BROWN FOX"};
char *psarr[4]
{
  "A message from our C++ program",
  "<span style=\"color:red\">A red text</span>",
  "<b>Bold</b> word",
  "As seen above strings can contain embedded HTML"
};

double pi = M_PI;

httpd       ui_server;      //HTTP server for user interface

//Data dictionary for user interface
JSD_STARTDIC (uivars)
  JSD_OBJECT ("sample"),
    JSD (hvar, JT_SHORT, 1, 0),
    JSD (huvar, JT_USHORT, 1, 0),
    JSD (ivar, JT_INT, 1, 0),
    JSD (iuvar, JT_UINT, 1, 0),
    JSD (lvar, JT_LONG, 1, 0),
    JSD (luvar, JT_ULONG, 1, 0),
    JSD (fvar, JT_FLT, 1, 0),
    JSD (dvar, JT_DBL, 1, 0),
    JSD (pstr, JT_PSTR, 1, sizeof (str1)),
    JSD (str, JT_STR, 1, sizeof (str)),
    JSD (bvar, JT_BOOL, 1, 0),
    JSD (iarr, JT_INT, _countof (iarr), 0),
    JSD (sarr, JT_STR, _countof (sarr), sizeof (sarr[0])),
    JSD (psarr, JT_PSTR, _countof (psarr), 0),
  JSD_ENDOBJ,
  JSDN (pi, "varpi", JT_DBL, 1, 0), //a variable with a different 'external' name
JSD_ENDDIC;

/*
Declare a JSON bridge to "var" location using 'uivars' dictionary.

That means every GET request to "http://server/var?xxx" will trigger a search
for the variable xxx in the 'uivars' JSON dictionary and the content of that
variable will be formatted as a JSON string and sent back to the client.
*/
JSONBridge  user_interface ("var", uivars);

//Assets for HTTP server
struct assetid {
  const char *name;
  int id;
  string fname;
} assets[] = {
  { "index.html", IDR_INDEX_HTML },
//  { "jquery.js", IDR_JQUERY_JS },
  { "main.css", IDR_MAIN_CSS },
  { 0, 0 }
};

//prototypes
bool write_asset_file (const std::string& path, const std::string& name, int id, std::string& fname);


/*
  Main window procedure.

  This app is designed to live as an icon in the system tray with very few
  functions. It processes the following messages:

  WM_TRAYNOTIFY - Icon click notifications

  WM_COMMAND    - Response to application menu commands
  WM_DESTROY    - Post a quit message and return
*/
LRESULT WINAPI WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  int wmId, wmEvent;
  static HMENU menu = 0;
  static char balloon[256];
  char cmd[256];

  switch (message)
  {
  case WM_CREATE:
    menu = LoadMenu (hInst, MAKEINTRESOURCE (IDM_UISAMPLE));

  case WM_COMMAND:
    wmId = LOWORD (wParam);
    wmEvent = HIWORD (wParam);
    // Parse the menu selections:
    switch (wmId)
    {
    case ID_OPENINTERFACE:
      sprintf_s (cmd, "http://localhost:%d", SERVER_PORT);
      ShellExecute (0, L"open", utf8::widen (cmd).c_str (), L"", L".", SW_SHOW);
      break;

    case ID_SAMPLE_EXIT:
      DestroyWindow (hWnd);
      break;
    default:
      return DefWindowProc (hWnd, message, wParam, lParam);
    }
    break;

  case WM_TRAYNOTIFY:
    switch (lParam)
    {
    case WM_LBUTTONDOWN:
      POINT pt;
      GetCursorPos (&pt);
      SetForegroundWindow (hWnd);
      TrackPopupMenuEx (GetSubMenu (menu, 0),
                        TPM_LEFTALIGN | TPM_LEFTBUTTON,
                        pt.x,
                        pt.y,
                        hWnd,
                        NULL
                        );
      PostMessage (hWnd, WM_NULL, 0, 0); //per MS KB Q135788
      break;

    case WM_LBUTTONDBLCLK:
      PostMessage (hWnd, WM_COMMAND, ID_OPENINTERFACE, 0l);
      break;

    case WM_CONTEXTMENU:
      //will put here a 2nd menu if needed
      break;
    }
    break;

  case WM_DESTROY:
    DestroyMenu (menu);
    nid.uFlags = NIF_ICON;
    Shell_NotifyIcon (NIM_DELETE, &nid);
    PostQuitMessage (0);
    break;

  default:
    return DefWindowProc (hWnd, message, wParam, lParam);
  }
  return 0;
}

/*
  Entry point.
*/
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
  MSG msg;
  HWND prevWnd;
  const wchar_t *app_title = L"Sample User Interface";

  /* Check if another instance is already running. If so, send an ID_OPENINTERFACE
  message to it. That makes it open a browser window showing the user interface.*/
  if ((prevWnd = FindWindow (SERVER_WNDCLASSNAME, 0)))
  {
    PostMessage (prevWnd, WM_COMMAND, ID_OPENINTERFACE, 0);
    return 0;   //already running
  }

  // Initialize globals
  hInst = hInstance;

  /*Find a temp folder for all HTML assets (the docroot)*/
  utf8::buffer tmp(_MAX_PATH);
  GetTempPathW (_MAX_PATH, tmp);
  docroot = tmp;
  docroot += "uisample";
  utf8::mkdir (docroot);

  /*Expand all assets in temp folder*/
  assetid *asset = assets;
  while (asset->name)
  {
    write_asset_file (docroot, asset->name, asset->id, asset->fname);
    asset++;
  }

  //Configure and start UI server
  ui_server.docroot (docroot.c_str());
  ui_server.port (SERVER_PORT);
  user_interface.attach_to (ui_server);
  ui_server.start ();

  //Register main window class
  WNDCLASSEX wcex;
  memset (&wcex, 0, sizeof (WNDCLASSEX));
  wcex.cbSize = sizeof (WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.hInstance = hInstance;
  wcex.hIcon = (HICON)LoadImage (hInstance, MAKEINTRESOURCE (IDI_UISAMPLE), IMAGE_ICON,
                                 ::GetSystemMetrics (SM_CXICON),
                                 ::GetSystemMetrics (SM_CYICON), 0);
  wcex.hCursor = LoadCursor (NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCE (IDM_UISAMPLE);
  wcex.lpszClassName = SERVER_WNDCLASSNAME;
  wcex.hIconSm = (HICON)LoadImage (wcex.hInstance, MAKEINTRESOURCE (IDI_UISAMPLE), IMAGE_ICON,
                                   ::GetSystemMetrics (SM_CXSMICON),
                                   ::GetSystemMetrics (SM_CYSMICON), 0);
  if (!RegisterClassEx (&wcex))
  {
    TRACE ("RegisterClassEx failed (%d)", GetLastError ());
    return 1;
  }

  //Create main window
  mainWnd = CreateWindowEx (
    0,                        //exStyle
    SERVER_WNDCLASSNAME,      //classname
    app_title,                //title
    WS_POPUP,                 //style
    CW_USEDEFAULT,            //X
    0,                        //Y
    CW_USEDEFAULT,            //W
    0,                        //H
    HWND_MESSAGE,             //parent
    NULL,                     //menu
    hInstance,                //instance
    NULL);                    //param
  if (!mainWnd)
  {
    TRACE ("Failed to create main window (%d)", GetLastError ());
    return 1;
  }

  //Create tray icon
  memset (&nid, 0, sizeof (NOTIFYICONDATA));
  nid.cbSize = sizeof (NOTIFYICONDATA);
  nid.hIcon = wcex.hIconSm;
  nid.hWnd = mainWnd;
  nid.uCallbackMessage = WM_TRAYNOTIFY;

  nid.uVersion = NOTIFYICON_VERSION_4;
  nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  wcsncpy_s (nid.szTip, app_title, _countof (nid.szTip)); // Copy tooltip
  wcsncpy_s (nid.szInfoTitle, app_title, _countof (nid.szInfoTitle));
  Shell_NotifyIcon (NIM_ADD, &nid);
  
  //run message pump
  try {
    PostMessage (mainWnd, WM_COMMAND, ID_OPENINTERFACE, 0);

    while (true)
    {
      while (GetMessage (&msg, NULL, 0, 0))
      {
        if (msg.message == WM_QUIT)
          break;
        TranslateMessage (&msg);
        DispatchMessage (&msg);
      }
      if (msg.message == WM_QUIT)
        break;
    }
  }
  catch (mlib::erc& e)
  {
    TRACE ("Error %s-%d", e.facility ().name (), e.code ());
  }

  //Delete tray icon
  Shell_NotifyIcon (NIM_DELETE, &nid);

  //Terminate UI server
  ui_server.terminate ();

  //Delete all assets from temp folder
  asset = assets;
  while (asset->name)
  {
    utf8::remove (asset->fname);
    asset++;
  }
  utf8::rmdir (docroot);
  return (int)msg.wParam;
}

//Load a resource in memory
static void *mem_resource (int name, int type, DWORD& size)
{
  HMODULE handle = GetModuleHandle (NULL);
  HRSRC rc = FindResource (handle, MAKEINTRESOURCE (name),
                             MAKEINTRESOURCE (type));
  if (!rc)
    return NULL;

  HGLOBAL rcData = LoadResource (handle, rc);
  if (!rcData)
    return NULL;
  size = SizeofResource (handle, rc);
  return LockResource (rcData);
}

/*!
  This function provides a nice mechanism for 'hiding' the different
  assets needed by the HTTP server (pages, CSS files, images, etc.) inside the
  EXE file.

  Each asset is stored as a user-defined resource of type TEXTFILE
  (defined as 256) and is identified by its ID. This function writes the asset
  to a file.

  \param  path  root path for all assets (with or without terminating backslash
  \param  name  asset filename (it can include a relative path)
  \param  id    asset id
  \param  fullpath  fullpath of asset file
  \return _true_ if successful
*/
bool write_asset_file (const std::string& path, const std::string& name, int id, std::string& fullpath)
{
  string tmp = path;
  int rc;

  fullpath.clear ();
  if (tmp.back () != '/' && tmp.back () != '\\'
   && name.front() != '/' && name.front() != '\\')
  {
    //Root path must be terminated with '\' (unless name starts with one)
    tmp.push_back ('\\');
  }

  //Make sure all folders on path exist. If not we create them now
  size_t idx = name.find_last_of ("/\\");
  if (idx != string::npos)
    tmp += name.substr (0, idx);
  if ((rc = r_mkdir (tmp)) && rc != EEXIST)
    return false; //could not create path

  //Load resource
  DWORD size = 0;
  void* data = mem_resource (id, TEXTFILE, size);         //load resource...
  if (!data)
    return false;
  if (idx != string::npos)
    tmp += name.substr (idx + 1);
  else
    tmp += name;
  FILE *f;
  f = utf8::fopen (tmp, "wb");        //... and write it
  if (!f)
    return false;
  fullpath = tmp;
  TRACE ("Writing resource size %d file %s", size, fullpath.c_str());
  fwrite (data, sizeof (char), size, f);
  fclose (f);
  return true;
}

/*!
  UISAMPLE.CPP - This program shows how to build a HTML user interface
  using the HTTP server and the JSONBridge interface.

  The program starts a HTTP server on a dynamically assigned port and opens a
  browser window to that address. Then, it continues living as a small icon in
  the systray. To end the program right click on the systray icon and select
  "Exit".

  (c) Mircea Neacsu 2017-2022

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

#include <mlib/jbridge.h>
#include <utf8/utf8.h>
#include <assert.h>
#include <mlib/asset.h>
#include <mlib/trace.h>

#include "resource.h"

using namespace std;
using namespace mlib;

//Number to string conversion macro
#define STR(X) #X
#define STRINGIZE(X) STR(X)

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
int ivar = -12345678;
unsigned int iuvar = 12345678;
long lvar = -12345678;
unsigned long luvar = 12345678;
float fvar = 123.45f;
double dvar = 123.45;
string str{ "Another C++ string that can have any length" };
bool bvar;
int iarr[4] {111, 222, 333, 444};
char sarr[4][80] {"THE", "THE QUICK", "THE QUICK BROWN", "THE QUICK BROWN FOX"};
const char *psarr[]
{
  "A message from our C++ program",
  "<span style=\"color:red\">A red text</span>",
  "<b>Bold</b> word",
  "As seen above, strings can contain embedded HTML"
};

double pi = atan(1)*4.;

httpd       ui_server;      //HTTP server for user interface
unsigned short server_port;

int submit_sarr (const std::string& uri, JSONBridge& ui);
int exit_server (const std::string& uri, JSONBridge& ui);


/*
  Declare a JSON bridge to "var" location.

  That means every GET request to "http://server/var?xxx" will trigger a search
  for the variable xxx and the content of that variable will be formatted as a
  JSON string and sent back to the client.
*/
JSONBridge  user_interface ("var");

//Assets for HTTP server

std::vector<asset> assets {
  {IDR_INDEX_HTML   ,"index.html"   },
  {IDR_ABOUT_HTML   ,"about.html"   },
  {IDR_FAVICON_ICO  ,"favicon.ico"  },
//{IDR_JQUERY_JS    ,"jquery.js"    },
  {IDR_MAIN_CSS     ,"css/main.css" },
};

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

  switch (message)
  {
  case WM_CREATE:
    menu = LoadMenu (hInst, MAKEINTRESOURCE (IDM_UISAMPLE));
    break;

  case WM_COMMAND:
    wmId = LOWORD (wParam);
    wmEvent = HIWORD (wParam);
    // Parse the menu selections:
    switch (wmId)
    {
    case ID_OPENINTERFACE:
      utf8::ShellExecute ("http://localhost:" + to_string(server_port));
      break;
    case ID_SAMPLE_ABOUT:
      utf8::ShellExecute ("http://localhost:" + to_string (server_port) + "/about.html");
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
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
  MSG msg;
  HWND prevWnd;
  const wchar_t *app_title = L"Sample User Interface";

  /* Check if another instance is already running. If so, send an ID_OPENINTERFACE
  message to it. That makes it open a browser window showing the user interface.*/
  if ((prevWnd = FindWindow (SERVER_WNDCLASSNAME, 0)))
  {
    PostMessage (prevWnd, WM_COMMAND, ID_OPENINTERFACE, 0);
    return 0;   //already running; just show the interface
  }

  // Initialize globals
  hInst = hInstance;

  /*Find a temp folder for all HTML assets (the docroot)*/
  docroot = utf8::GetTempPath() + "uisample";
  utf8::mkdir (docroot);

  /*Expand all assets in temp folder*/
  for (auto& a : assets)
    a.write (docroot);

  //Configure UI server
  ui_server.docroot (docroot.c_str());

  // Populate UI variables
  auto& sample = user_interface.add_object ("sample");
  sample.add_var (iarr, "iarr");
  sample.add_var (hvar, "hvar");
  sample.add_var (huvar, "huvar");
  sample.add_var (ivar, "ivar");
  sample.add_var (iuvar, "iuvar");
  sample.add_var (lvar, "lvar");
  sample.add_var (luvar, "luvar");
  sample.add_var (fvar, "fvar");
  sample.add_var (dvar, "dvar");
  sample.add_var (str1, "str1");
  sample.add_var (str, "str");
  sample.add_var (sarr, "sarr");
  sample.add_var (bvar, "bvar");
  sample.add_var (psarr, "psarr");

  user_interface.add_var (pi, "varpi");

  user_interface.add_postfun ("submit_sarr", submit_sarr);
  user_interface.add_postfun ("exit_server", exit_server);

  //Attach the "JSON bridge" to server
  user_interface.attach_to (ui_server);

  //Set action after receiving user data
  user_interface.set_action ([](JSONBridge& ui) {
    ui.client ()->redirect ("/");
    });
  //Start the server
  ui_server.start ();

  server_port = ui_server.socket ().name ()->port ();


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
  for (auto& a : assets)
    a.remove ();

  utf8::rmdir (docroot);
  return (int)msg.wParam;
}

/*
  A function called through the JT_POSTFUN mechanism.

  The corresponding entry in our JSON dictionary is:
    JSD (submit_sarr, JT_POSTFUN, 0, 0),

  The function is invoked in response to a POST request like this:
    POST /var?submit_sarr HTTP/1.1
    Host: localhost:8080
    Connection: keep-alive
    Content-Length: 79
    Pragma: no-cache
    .... (other headers)

    sarr_0=THE&sarr_1=THE+QUICK&sarr_2=THE+QUICK+BROWN&sarr_3=THE+QUICK+BROWN+FOX

  It calls the parse_urlencoded function to retrieve the latest values
  and then shows a message box with the new values of the sarr array.
*/
int submit_sarr (const std::string& uri, JSONBridge& ui)
{
  bool ok = ui.parse_urlencoded ();
  string mbox_msg = 
    "sarr[0] " + string (sarr[0]) + "\n"
    "sarr[1] " + string (sarr[1]) + "\n"
    "sarr[2] " + string (sarr[2]) + "\n"
    "sarr[3] " + string (sarr[3]) + "\n";

  /* The function is executed in the context of connection specific thread,
  not the main UI thread. If function needs to do more UI stuff, a nicer
  approach would be to post a message to main UI thread and let it do the work.
  
  This is just a quick and dirty handler.*/
  utf8::MessageBox (mainWnd, mbox_msg, "UI Sample App", MB_OK | MB_SYSTEMMODAL);

  ui.client ()->add_ohdr ("Connection", "Close");
  return 0;
}

int exit_server (const std::string& uri, JSONBridge& ui)
{
  PostMessage (mainWnd, WM_COMMAND, ID_SAMPLE_EXIT, 0);
  ui.client ()->add_ohdr ("Connection", "Close");
  return 0;
}


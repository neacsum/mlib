#include <windows.h>

#include <mlib/utf8.h>
#include <vector>

using namespace std;

string narrow(const wchar_t *s)
{
  int wsz = (int)wcslen(s);
  int nsz = WideCharToMultiByte (CP_UTF8, 0, s, wsz, 0, 0, 0, 0);
  string out (nsz, 0);
  if (nsz)
    WideCharToMultiByte (CP_UTF8, 0, s, wsz, &out[0], nsz, 0, 0);
  return out;
}

wstring widen(const char *s)
{
  int nsz = (int)strlen (s);
  int wsz = MultiByteToWideChar (CP_UTF8, 0, s, nsz, 0, 0);
  wstring out (wsz, 0);
  if (wsz)
    MultiByteToWideChar (CP_UTF8, 0, s, nsz, &out[0], wsz);
  return out;
}

string narrow(const wstring &s)
{
  int wsz = (int)s.size();
  int nsz = WideCharToMultiByte (CP_UTF8, 0, &s[0], wsz, 0, 0, 0, 0);
  string out (nsz, 0);
  if (nsz)
    WideCharToMultiByte (CP_UTF8, 0, &s[0], wsz, &out[0], nsz, 0, 0);
  return out;
}

wstring widen(const string &s)
{
  int nsz = (int)s.size ();
  int wsz = MultiByteToWideChar (CP_UTF8, 0, &s[0], nsz, 0, 0);
  wstring out (wsz, 0);
  if (wsz)
    MultiByteToWideChar (CP_UTF8, 0, &s[0], nsz, &out[0], wsz);
  return out;
}
  
char **get_utf8argv (int *argc)
{
  wchar_t **wargv = CommandLineToArgvW ( GetCommandLineW(), argc);
  char** uargv = (char **)malloc (*argc*sizeof(char*));
  for (int i=0; i<*argc; i++)
  {
    int nc = WideCharToMultiByte (CP_UTF8, 0, wargv[i], -1, 0, 0, 0, 0);
    uargv[i] = (char *)malloc (nc+1);
    WideCharToMultiByte (CP_UTF8, 0, wargv[i], -1, uargv[i], nc, 0, 0);    
  }
  LocalFree (wargv);
  return uargv;
}

vector<string> get_utf8argv ()
{
  int argc;
  vector<string> uargv;

  wchar_t **wargv = CommandLineToArgvW (GetCommandLineW(), &argc);
  for (int i=0; i<argc; i++)
    uargv.push_back (narrow (wargv[i]));
  LocalFree (wargv);
  return uargv;
}

void free_utf8argv (int argc, char **argv)
{
  for (int i=0; i<argc; i++)
    free (argv[i]);
  free (argv);
}
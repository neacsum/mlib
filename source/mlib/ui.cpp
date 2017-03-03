/*!
  \file   ui.cpp
  (c) Mircea Neacsu 2017

  Implementation of ui_context class
*/
#include <memory.h>
#include <string.h>

#include <mlib/ui.h>
#include <mlib/trace.h>
#include <mlib/critsect.h>

using namespace std;

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// The JSON data dictionary
extern JSONVAR json_dict[];

#define MAX_JSONRESPONSE 8192

// Sizes of data elements (same order as JT_.. values)
const int jsz[] = { sizeof (short int), sizeof (unsigned short int), 
sizeof (int), sizeof (unsigned int), sizeof (long), sizeof (unsigned long), 
sizeof (float), sizeof (double), sizeof (char*), sizeof(char) };


static int url_decode (const char *in, char *out);
static int hexdigit (char *bin, char c);
static int hexbyte (char *bin, const char *str);

ui_context::ui_context (const char *path) :
path_ (path)
{
  buffer = new char[MAX_JSONRESPONSE];
}

ui_context::~ui_context ()
{
  delete buffer;
}

void ui_context::attach_to (httpd& server)
{
  server.add_handler (path_, (uri_handler)ui_context::callback, this);
}

void ui_context::json_begin (http_connection* cl)
{
  TRACE ("ui_context::json_begin");
  in_use.enter ();
  bufptr = buffer;
  *bufptr++ = '{';
  client = cl;
}

/// Send out the JSON formatted buffer
void ui_context::json_end ()
{
  if (',' == *(bufptr - 1))
    bufptr--; //kill the trailing comma 

  strcpy (bufptr, "}\r\n");
  bufptr += 3;
  client->out ()
    << "HTTP/1.1 200 OK\r\n"
       "Cache-Control: no-cache, no-store\r\n"
       "Content-Type: text/plain\r\n"
     //"Access-Control-Allow-Origin: *\r\n"
       "Connection: Keep-Alive\r\n"
       "Content-Length: " << (bufptr - buffer) << "\r\n"
       "\r\n"
    << buffer
    << endl;
  in_use.leave ();
}

/// Serializes a variable to JSON format
bool ui_context::jsonify (void *var)
{
  const JSONVAR *entry;
  int i;
  char *addr = (char *)var;

  for (entry = json_dict; entry->name && entry->addr != addr; entry++)
    ;

  if (!entry->name)
  {
    TRACE ("Missing dictionary entry!!");
    return false;   //oops! entry not found
  }

  *bufptr++ = '"';
  strcpy (bufptr, entry->name);
  bufptr += strlen (bufptr);
  *bufptr++ = '"';
  *bufptr++ = ':';

  if (entry->cnt > 1)
    *bufptr++ = '[';

  i = 0;
  while (i<entry->cnt)
  {
    switch (entry->type)
    {
    case JT_PSTR:
      sprintf (bufptr, "\"%s\"", *(char**)addr);
      break;
    case JT_STR:
      sprintf (bufptr, "\"%s\"", addr);
      break;
    case JT_SHORT:
      sprintf (bufptr, "%hd", *(short int*)addr);
      break;
    case JT_USHORT:
      sprintf (bufptr, "%hu", *(unsigned short int*)addr);
      break;
    case JT_INT:
      sprintf (bufptr, "%d", *(int*)addr);
      break;
    case JT_UINT:
      sprintf (bufptr, "%u", *(unsigned int*)addr);
      break;
    case JT_LONG:
      sprintf (bufptr, "%ld", *(long*)addr);
      break;
    case JT_ULONG:
      sprintf (bufptr, "%lu", *(unsigned long*)addr);
      break;
    case JT_FLT:
      sprintf (bufptr, "%.10g", *(float*)addr);
    case JT_DBL:
      sprintf (bufptr, "%.10lg", *(double*)addr);
      break;
    }
    bufptr += strlen (bufptr);
    i++;
    if (i < entry->cnt)
      *bufptr++ = ',';
    if (entry->type == JT_STR)
      addr += entry->sz;
    else
      addr += jsz[entry->type];
  }
  if (entry->cnt > 1)
    *bufptr++ = ']';

  *bufptr++ = ',';
  return true;
}

/// Allow derived classes to generate response inside JSON buffer
void ui_context::bprintf (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  bufptr += vsprintf (bufptr, fmt, args);
}

void ui_context::not_found (const char *varname)
{
  char tmp[1024];
  sprintf (tmp, "HTTP/1.1 415 Unknown variable %s\r\n", varname);
  client->out() << "HTTP/1.1 415 Unknown variable " << varname << "\r\n"
    << "Content-Type: text/plain\r\n"
    << "Content-Length: " << strlen (tmp) << "\r\n"
    << "\r\n"
    << tmp
    << endl;
}

/*!
  Search a variable in JSON dictionary. The variable name can be a construct
  '<name>_<index>' for an indexed variable.
*/
const JSONVAR* ui_context::find (const char *name, int *idx)
{
  const JSONVAR *entry;
  char varname[256];
  char *pi = strchr (varname, '_'), *tail;

  strncpy (varname, name, sizeof(varname)-1);
  varname[sizeof (varname) - 1] = 0;
  *idx = 0;
  if (pi)
  {
    *idx = (int)strtol (pi + 1, &tail, 10);
    if (*tail == 0 && *idx >= 0) //if name is indeed <var>_<number> and index is positive...
      *pi = 0;                   //...search dictionary for <var>
    else
      *idx = 0;
  }

  for (entry = json_dict; entry->name; entry++)
  {
    if (!strcmp (varname, entry->name))
    {
      if (*idx < entry->cnt)
        return entry;
      return NULL;
    }
  }
  return NULL;
}

/*!
  Parse the URL-encoded body of a POST request assigning new values to all
  variables.
*/
bool ui_context::parse_urlencoded (http_connection* cl)
{
  char val[1024];
  int idx;
  void *pv;

  mlib::lock l (in_use);
  client = cl;
  str_pairs vars;
  cl->parse_formbody (vars);
  for (auto var = vars.begin (); var != vars.end (); var++)
  {
    if (!var->second.length ())
      continue;     // missing '=value' part of 'key=value' construct

    strcpy (val, var->second.c_str ());
    const JSONVAR *k = find (var->first.c_str (), &idx);
    if (!k)
      continue;     //variable not found
    url_decode (val, val);
    if (k->type == JT_STR)
      pv = (char*)(k->addr) + k->sz*idx;
    else
      pv = (char*)(k->addr) + jsz[k->type] * idx;

    TRACE("Setting %s[%d] = %s\n", k->name, idx, val);
    switch (k->type)
    {
    case JT_PSTR:
      pv = *(char**)pv; //one more level of indirection
      //flow through to JT_STR case. Don't break them appart!
    case JT_STR:
      strncpy ((char *)pv, val, k->sz);
      if (k->sz)
        *((char *)pv + k->sz - 1) = 0; //always null-terminated
      break;
    case JT_INT:
      *(int*)pv = (int)strtol (val, NULL, 0);
      break;
    case JT_UINT:
      *(unsigned int*)pv = (unsigned int)atol (val);
      break;
    case JT_SHORT:
      *(short*)pv = (short)strtol (val, NULL, 0);
      break;
    case JT_USHORT:
      *(unsigned short*)pv = (unsigned short)atol (val);
      break;
    case JT_LONG:
      *(long *)pv = strtol (val, NULL, 0);
      break;
    case JT_ULONG:
      *(unsigned long *)pv = atol (val);
      break;
    case JT_FLT:
      *(float *)pv = (float)atof (val);
    case JT_DBL:
      *(double *)pv = atof (val);
      break;
    }
  }
  post_parse (cl->get_query());
  return true;
}


int ui_context::callback (const char *uri, http_connection& client, ui_context *ctx)
{
  const char *req = client.get_method ();
  const char *query = client.get_query ();
  TRACE ("ui_callback req=%s query=%s", req, query);
  if (!_strcmpi (req, "GET"))
  {
    int ret;
    ctx->json_begin (&client);
    if (200 == (ret = ctx->jsonify_all (query)))
      ctx->json_end ();
    else if (ret == 404)
      client.respond (404);
  }
  else if (!_strcmpi (req, "POST"))
  {
    if (!_strcmpi (client.get_ihdr ("Content-Type"), "application/x-www-form-urlencoded"))
      ctx->parse_urlencoded (&client);
  }
  return 1;
}

int hexdigit (char *bin, char c)
{
  c = toupper (c);
  if (c >= '0' && c <= '9')
    *bin = c - '0';
  else if (c >= 'A' && c <= 'F')
    *bin = c - 'A' + 10;
  else
    return 0;

  return 1;
}

int hexbyte (char *bin, const char *str)
{
  char d1, d2;

  //first digit
  if (!hexdigit (&d1, *str++) || !hexdigit (&d2, *str++))
    return 0;
  *bin = (d1 << 4) | d2;
  return 1;
}

/*!
Decoding of URL-encoded data.
We can do it in place because resulting string is shorter or equal than input.

\return 1 if successful, 0 otherwise
*/
int url_decode (const char *in, char *out)
{
  while (*in)
  {
    if (*in == '%')
    {
      if (!hexbyte (out++, ++in))
        return 0;
      in++;
    }
    else if (*in == '+')
      *out++ = ' ';
    else
      *out++ = *in;
    in++;
  }
  *out = 0;
  return 1;
}

#ifdef MLIBSPACE
}
#endif

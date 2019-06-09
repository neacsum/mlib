/*!
  \file   jbridge.cpp Implementation of JSONBridge class

  (c) Mircea Neacsu 2017
*/
#include <memory.h>
#include <string.h>

#include <mlib/jbridge.h>
#include <mlib/trace.h>
#include <mlib/critsect.h>

using namespace std;

/// The JSON data dictionary
extern mlib::JSONVAR json_dict[];

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

#define MAX_JSONRESPONSE 8192

// Sizes of data elements (same order as JT_.. values)
const int jsz[] = { 
  sizeof (short int),             //JT_SHORT
  sizeof (unsigned short int),    //JT_USHORT
  sizeof (int),                   //JT_INT
  sizeof (unsigned int),          //JT_UINT
  sizeof (long),                  //JT_LONG
  sizeof (unsigned long),         //JT_ULONG
  sizeof (float),                 //JT_FLT
  sizeof (double),                //JT_DBL
  sizeof (char*),                 //JT_PSTR
  sizeof(char),                   //JT_STR
  sizeof(bool)                    //JT_BOOL
};


static int url_decode (const char *in, char *out);
static int hexdigit (char *bin, char c);
static int hexbyte (char *bin, const char *str);
/*!
  \class JSONBridge
  \ingroup sockets

  JSONBridge class provides an easy access mechanism to program variables for
  HTTP server. A JSONBridge object is attached to a HTTP server using the
  attach_to() function. Behind the scene, this function registers a handler function
  for the path associated with the JSONBridge. 
  
  Assume the HTTP server hs answers requests sent to http://localhost:8080 and 
  the JSONBridge object was created
  as:
  \code
    JSONBridge jb("var");
  \endcode

  Then:
  \code
    jb.attach_to(hs);
  \endcode
  will register a handler for requests to http://localhost:8080/var

  If this handler is invoked with a GET request for http://localhost:8080/var?data
  it will search in the JSON dictionary a variable called 'data' and return it's
  content as a JSON object.
*/

///Creates a JSONBridge object for the given path
JSONBridge::JSONBridge (const char *path)
  : path_ (path)
  , client_ (0)
{
  buffer = new char[MAX_JSONRESPONSE];
}

JSONBridge::~JSONBridge ()
{
  delete buffer;
}

/// Attach JSONBridge object to a HTTP server
void JSONBridge::attach_to (httpd& server)
{
  server.add_handler (path_, (uri_handler)JSONBridge::callback, this);
}

/// Initializes the response buffer 
bool JSONBridge::json_begin ()
{
  TRACE ("JSONBridge::json_begin");
  in_use.enter ();
  int idx;
  const JSONVAR *entry = find (client().get_query (), &idx);
  if (!entry)
    return false;
  bufptr = buffer;
  *bufptr++ = '{';
  if (entry->type == JT_OBJECT)
  {
    entry++;
    while (entry->type != JT_ENDOBJ)
      jsonify (entry);
  }
  else if (!jsonify (entry))
    return false;
  json_end ();
  return true;
}

/// Send out the JSON formatted buffer
void JSONBridge::json_end ()
{
  if (',' == *(bufptr - 1))
    bufptr--; //kill the trailing comma 

  strcpy (bufptr, "}\r\n");
  bufptr += 3;
  client().out ()
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
bool JSONBridge::jsonify (const JSONVAR*& entry)
{
  *bufptr++ = '"';
  strcpy (bufptr, entry->name);
  bufptr += strlen (bufptr);
  *bufptr++ = '"';
  *bufptr++ = ':';

  if (entry->cnt > 1)
    *bufptr++ = '[';

  int i = 0;
  char *addr = (char*)entry->addr;

  while (i<entry->cnt)
  {
    switch (entry->type)
    {
    case JT_PSTR:
      strquote (*(char**)addr);
      break;
    case JT_STR:
      strquote (addr);
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
      break;
    case JT_DBL:
      sprintf (bufptr, "%.10lg", *(double*)addr);
      break;
    case JT_BOOL:
      strcpy (bufptr, *(bool*)addr ? "true" : "false");
      break;
    case JT_OBJECT:
      *bufptr++ = '{';
      entry++;
      while (entry->type != JT_ENDOBJ)
        jsonify (entry);

      *(bufptr-1) = '}'; //replace ending comma with closing brace
      *bufptr = 0;
      break;

    default:
      TRACE ("Unexpected entry type %d", entry->type);
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
  *bufptr = 0; //null terminated just to play safe
  entry++; //advance to next dictionary entry
  return true;
}

/// Allow derived classes to generate response inside JSON buffer
void JSONBridge::bprintf (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  bufptr += vsprintf (bufptr, fmt, args);
}

void JSONBridge::not_found (const char *varname)
{
  char tmp[1024];
  sprintf (tmp, "HTTP/1.1 415 Unknown variable %s\r\n", varname);
  client().out() << "HTTP/1.1 415 Unknown variable " << varname << "\r\n"
    << "Content-Type: text/plain\r\n"
    << "Content-Length: " << strlen (tmp) << "\r\n"
    << "\r\n"
    << tmp
    << endl;
}

bool JSONBridge::strquote (const char *str)
{
  //trivial null case
  if (!str)
  {
    strcpy (bufptr, "null");
    bufptr += 4;
    return true;
  }

  *bufptr++ = '"';
  while (*str && bufptr - buffer < MAX_JSONRESPONSE)
  {
    const char repl[] = "\"\\/bfnrt";
    const char quote[] = "\"\\/\b\f\n\r\t";
    const char *ptr;
    if (ptr = strchr (quote, *str))
    {
      size_t i = ptr - quote;
      *bufptr++ = '\\';
      *bufptr++ = repl[i];
      str++;
    }
    else
      *bufptr++ = *str++;
  }
  if (*str)
    return false; //buffer full
  *bufptr++ = '"';
  *bufptr = 0;
  return true;
}
/*!
  Search a variable in JSON dictionary. The variable name can be a construct
  '<name>_<index>' for an indexed variable.
*/
JSONVAR* JSONBridge::find (const char *name, int *pidx)
{
  JSONVAR *entry;
  char varname[256];
  char *pi, *tail;
  int idx;
  strncpy (varname, name, sizeof(varname)-1);
  varname[sizeof (varname) - 1] = 0;
  pi = strchr (varname, '_');
  if (!pidx)
    pidx = &idx;
  *pidx = 0;
  if (pi)
  {
    *pidx = (int)strtol (pi + 1, &tail, 10);
    if (*tail == 0 && *pidx >= 0) //if name is indeed <var>_<number> and index is positive...
      *pi = 0;                   //...search dictionary for <var>
    else
      *pidx = 0;
  }

  for (entry = json_dict; entry->name; entry++)
  {
    if (!strcmp (varname, entry->name))
    {
      if (*pidx < entry->cnt)
        return entry;
      TRACE ("Out of bounds %s[%d]", varname, *pidx);
      return NULL;
    }
  }
  return NULL;
}

/// Set/change address of a dictionary entry
bool JSONBridge::set_var (const char *name, void *addr, unsigned short count, unsigned short sz)
{
  int idx;
  JSONVAR *entry = find (name, &idx);
  if (!entry || idx != 0)
    return false;

  entry->addr = addr;
  entry->cnt = count;
  entry->sz = sz;
  return true;
}

/*!
  Parse the URL-encoded body of a POST request assigning new values to all
  variables.
*/
bool JSONBridge::parse_urlencoded ()
{
  char val[1024];
  int idx;
  void *pv;

  mlib::lock l (in_use);
  str_pairs vars;
  parse_urlparams (client().get_body (), vars);
  for (auto var = vars.begin (); var != vars.end (); var++)
  {
    if (!var->second.length ())
      continue;     // missing '=value' part of 'key=value' construct

    strcpy (val, var->second.c_str ());
    const JSONVAR *k = find (var->first.c_str (), &idx);
    if (!k)
    {
      TRACE ("Posted key %s not found in dictionary", var->first.c_str ());
      continue;
    }
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
      //flow through to JT_STR case. Don't break them apart!
    case JT_STR:
      strncpy ((char *)pv, val, k->sz);
      if (k->sz)
        *((char *)pv + k->sz - 1) = 0; //always null-terminated
      break;
    case JT_INT:
      *(int*)pv = (int)strtol (val, NULL, 0);
      break;
    case JT_UINT:
      *(unsigned int*)pv = (unsigned int)strtoul (val, NULL, 0);
      break;
    case JT_SHORT:
      *(short*)pv = (short)strtol (val, NULL, 0);
      break;
    case JT_USHORT:
      *(unsigned short*)pv = (unsigned short)strtoul (val, NULL, 0);
      break;
    case JT_LONG:
      *(long *)pv = strtol (val, NULL, 0);
      break;
    case JT_ULONG:
      *(unsigned long *)pv = strtoul (val, NULL, 0);
      break;
    case JT_FLT:
      *(float *)pv = (float)atof (val);
      break;
    case JT_DBL:
      *(double *)pv = atof (val);
      break;
    case JT_BOOL:
      *(bool *)pv = (atoi (val) != 0);
      break;
    default:
      TRACE ("Unexpected entry type: %d", k->type);
    }
  }
  return true;
}

/*!
  Redirect client to root
*/
void JSONBridge::post_parse ()
{
  client().redirect ("/");
}

int JSONBridge::callback (const char *uri, http_connection& client, JSONBridge *ctx)
{
  const char *req = client.get_method ();
  const char *query = client.get_query ();
  TRACE ("ui_callback req=%s query=%s", req, query);
  ctx->client_ = &client;
  if (!_strcmpi (req, "GET"))
  {
    int ret;
    if (200 == (ret = ctx->json_begin ()))
      ctx->json_end ();
    else if (ret == 404)
      client.respond (404);
  }
  else if (!_strcmpi (req, "POST"))
  {
    if (!_strcmpi (client.get_ihdr ("Content-Type"), "application/x-www-form-urlencoded"))
      ctx->parse_urlencoded ();
    const JSONVAR* entry;
    int idx;
    if (strlen (client.get_query ()) 
     && (entry = ctx->find (client.get_query (), &idx))
     && entry->type == JT_POSTFUN)
    {
      uri_handler(entry->addr)(uri, client, 0);
    }
    ctx->post_parse ();

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

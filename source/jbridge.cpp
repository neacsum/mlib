/*!
  \file   jbridge.cpp Implementation of JSONBridge class

  (c) Mircea Neacsu 2017
*/
#include <memory.h>
#include <string.h>
#include <sstream>

#include <mlib/jbridge.h>
#include <mlib/trace.h>
#include <mlib/critsect.h>
#include <mlib/json.h>
#include <algorithm>

using namespace std;


namespace mlib {

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
  sizeof(bool),                   //JT_BOOL
  0,                              //JT_OBJECT
  0,                              //JT_ENDOBJ
  0                               //JT_POSTFUN
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
JSONBridge::JSONBridge (const char *path, JSONVAR* dict)
  : path_ (path)
  , dictionary (dict)
  , client_ (0)
{
}

JSONBridge::~JSONBridge ()
{
}

/// Attach JSONBridge object to a HTTP server
void JSONBridge::attach_to (httpd& server)
{
  server.add_handler (path_.c_str(), (uri_handler)JSONBridge::callback, this);
}

///  
erc JSONBridge::json_begin (json::node& root)
{
  TRACE9 ("JSONBridge::json_begin - %s", client ().get_query ());
  mlib::lock l(in_use);
  int idx;
  const JSONVAR *entry = find (client().get_query (), &idx);
  if (!entry)
  {
    TRACE2 ("JSONBridge::json_begin - Cannot find %s", client ().get_query ());
    return erc (ERR_JSON_NOTFOUND, ERROR_PRI_ERROR);
  }
  return jsonify (root, entry);
}

/// Send out the JSON formatted buffer
erc JSONBridge::json_end (json::node& obj)
{
  try {
    stringstream ss;
    ss << fixed << obj;

    client ().out ()
      << "HTTP/1.1 200 OK\r\n"
      "Cache-Control: no-cache, no-store\r\n"
      "Content-Type: text/plain\r\n"
      //"Access-Control-Allow-Origin: *\r\n"
      "Connection: Keep-Alive\r\n"
      "Content-Length: " << ss.str ().size () << "\r\n"
      "\r\n"
      << ss.str ()
      << endl;
  }
  catch (erc& x) {
    x.reactivate ();
    return x;
  }
  return ERR_SUCCESS;
}

/// Serializes a variable to JSON format
erc JSONBridge::jsonify (json::node& n, const JSONVAR*& entry)
{
  try {
    if (entry->cnt > 1)
    {
      for (int i = 0; i < entry->cnt; i++)
        do_node (n[i], entry, i);
      return ERR_SUCCESS;
    }
    else
      return do_node (n, entry);
  }
  catch (erc& x) {
    x.reactivate ();
    return x;
  }
}

erc JSONBridge::do_node (json::node& n, const JSONVAR*& entry, int index)
{
  char* addr;
  if (entry->type == JT_STR)
    addr = (char*)entry->addr + entry->sz * index;
  else
    addr = (char*)entry->addr + jsz[entry->type] * index;

  switch (entry->type)
  {
  case JT_PSTR:
    n = *(char**)addr;
    break;
  case JT_STR:
    n = (char*)addr;
    break;
  case JT_SHORT:
    n = *(short int*)addr;
    break;
  case JT_USHORT:
    n = *(unsigned short int*)addr;
    break;
  case JT_INT:
    n = *(int*)addr;
    break;
  case JT_UINT:
    n = *(unsigned int*)addr;
    break;
  case JT_LONG:
    n = *(long*)addr;
    break;
  case JT_ULONG:
    n = *(unsigned long*)addr;
    break;
  case JT_FLT:
    n = *(float*)addr;
    break;
  case JT_DBL:
    n = *(double*)addr;
    break;
  case JT_BOOL:
    n = *(bool*)addr;
    break;
  case JT_OBJECT:
    //Composite object - go through dictionary 
    while (entry->name)
    {
      entry++;
      if (entry->type == JT_ENDOBJ)
        return ERR_SUCCESS;
      jsonify (n[entry->name], entry);
    }
    //end of dictionary
    return erc (ERR_JSON_DICSTRUC, ERROR_PRI_ERROR, json::errors);
    break;

  default:
    TRACE ("Unexpected entry type %d", entry->type);
    return erc (ERR_JSON_DICSTRUC, ERROR_PRI_ERROR, json::errors);
  }
  return ERR_SUCCESS;
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

/*!
  Search a variable in JSON dictionary. The variable name can be a construct
  `<name>_<index>` for an indexed variable.
*/
JSONVAR* JSONBridge::find (const char* name, int *pidx)
{
  JSONVAR *entry;
  char buf[256];
  int idx;

  if (!pidx)
    pidx = &idx;

  strncpy_s (buf, name, sizeof (buf)-1);
  buf[sizeof (buf) - 1] = 0;
  char* pi = strchr (buf, '_');
  *pidx = 0;

  if (pi)
  {
    char* tail;
    *pidx = strtol (pi + 1, &tail, 10);
    if (*tail || *pidx < 0) //if name doesn't match <var>_<number> or index is negative...
      *pidx = 0;            //...search dictionary for whole name
    else
      *pi = 0;
  }

  int lvl = 0;
  for (entry = dictionary; entry->name; entry++)
  {
    //search only top level entries
    if (entry->type == JT_OBJECT)
      lvl++;
    else if (entry->type == JT_ENDOBJ)
      lvl--;

    if (lvl <= 1 && !strcmp(buf, entry->name))
    {
      if (*pidx < entry->cnt)
        return entry;
      TRACE ("Out of bounds %s[%d]", buf, *pidx);
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

    TRACE9 ("Setting %s[%d] = %s\n", k->name, idx, val);
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

bool JSONBridge::parse_jsonencoded ()
{
  return false;
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
  TRACE9 ("ui_callback req=%s query=%s", req, query);
  ctx->client_ = &client;
  ctx->lock ();
  if (!_strcmpi (req, "GET"))
  {
    json::node root;
    if (ctx->json_begin (root) == ERR_SUCCESS)
      ctx->json_end (root);
    else
      client.respond (404);
  }
  else if (!_strcmpi (req, "POST"))
  {
    bool ok = false;
    std::string content = client.get_ihdr ("Content-Type");
    //make lower case
    std::transform (content.begin (), content.end (), content.begin(), 
      [] (char c)->char {return tolower (c); });


    const JSONVAR* entry;
    int idx;
    if (strlen (client.get_query ())
      && (entry = ctx->find (client.get_query (), &idx))
      && entry->type == JT_POSTFUN)
    {
      uri_handler (entry->addr)(uri, client, ctx);
      ok = true;
    }
    else if (content == "application/x-www-form-urlencoded")
      ok = ctx->parse_urlencoded ();
    else if (content.rfind ("application/json", 0) == 0)
      ok = ctx->parse_jsonencoded ();

    if (ok)
      ctx->post_parse ();
  }
  ctx->unlock ();
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

}

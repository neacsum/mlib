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
  
  Assume the HTTP server `hs` answers requests sent to `http://localhost:8080` and 
  the JSONBridge object was created
  as:
  \code
    JSONBridge jb("var");
  \endcode

  Then:
  \code
    jb.attach_to(hs);
  \endcode
  will register a handler for requests to `http://localhost:8080/var`

  If this handler is invoked with a GET request for http://localhost:8080/var?data
  it will search in the JSON dictionary a variable called 'data' and return it's
  content as a JSON object.
*/

///Creates a JSONBridge object for the given path
JSONBridge::JSONBridge (const char *path, jb_dictionary& dict)
  : path_ (path)
  , dict_ (dict)
  , client_ (nullptr)
  , action (nullptr)
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
  TRACE9 ("JSONBridge::json_begin - %s", client ()->get_query ());
  mlib::lock l(in_use);
  int idx;
  const jb_var *entry = find (client()->get_query (), &idx);
  if (!entry)
  {
    TRACE2 ("JSONBridge::json_begin - Cannot find %s", client ()->get_query ());
    return erc (ERR_JSON_NOTFOUND, ERROR_PRI_ERROR);
  }
  return jsonify (root, entry);
}

/// Send out the JSON formatted buffer
erc JSONBridge::json_end (json::node& obj)
{
  try {
    //serialize in a buffer to find content length
    stringstream ss;
    ss << fixed << obj;

    client ()->out ()
      << "HTTP/1.1 200 OK\r\n"
      "Cache-Control: no-cache, no-store\r\n"
      "Content-Type: application/json\r\n"
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
erc JSONBridge::jsonify (json::node& n, const jb_var*& entry)
{
  try {
    if (entry->cnt > 1)
    {
      for (int i = 0; i < entry->cnt; i++)
        serialize_node (n[i], entry, i);
      return ERR_SUCCESS;
    }
    else
      return serialize_node (n, entry);
  }
  catch (erc& x) {
    x.reactivate ();
    return x;
  }
}

erc JSONBridge::serialize_node (json::node& n, const jb_var*& entry, int index)
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
    n = (int) * (short int*)addr;
    break;
  case JT_USHORT:
    n = (int) * (unsigned short int*)addr;
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
    while (!entry->name.empty())
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

erc JSONBridge::deserialize_node (const json::node& n, const jb_var*& entry, int index) const
{
  void* pv;

  if (entry->type == JT_STR)
    pv = (char*)(entry->addr) + entry->sz * index;
  else
    pv = (char*)(entry->addr) + jsz[entry->type] * index;
  try {
    switch (entry->type)
    {
    case JT_PSTR:
      pv = *(char**)pv; //one more level of indirection
      //flow through to JT_STR case. Don't break them apart!
    case JT_STR:
      strncpy ((char*)pv, (const char*)n, entry->sz);
      if (entry->sz)
        *((char*)pv + entry->sz - 1) = 0; //always null-terminated
      break;
    case JT_INT:
      *(int*)pv = static_cast<int>(n);
      break;
    case JT_UINT:
      *(unsigned int*)pv = static_cast<int>(n);
      break;
    case JT_SHORT:
      *(short*)pv = static_cast<int>(n);
      break;
    case JT_USHORT:
      *(unsigned short*)pv = static_cast<int>(n);
      break;
    case JT_LONG:
      *(long*)pv = static_cast<int>(n);
      break;
    case JT_ULONG:
      *(unsigned long*)pv = static_cast<int>(n);
      break;
    case JT_FLT:
      *(float*)pv = (float)static_cast<double>(n);
      break;
    case JT_DBL:
      *(double*)pv = static_cast<double>(n);
      break;
    case JT_BOOL:
      *(bool*)pv = static_cast<bool>(n);
      break;
    default:
      TRACE ("Unexpected entry type: %d", entry->type);
    }
  }
  catch (mlib::erc& x) {
    x.reactivate ();
    return x;
  }
  return ERR_SUCCESS;
}

void JSONBridge::not_found (const char *varname)
{
  string tmp = "HTTP/1.1 415 Unknown variable %s\r\n";
  tmp += varname;

  client()->out() << "HTTP/1.1 415 Unknown variable " << varname << "\r\n"
    << "Content-Type: text/plain\r\n"
    << "Content-Length: " << tmp.size() << "\r\n\r\n"
    << tmp
    << endl;
}

/*!
  Search a variable in JSON dictionary. The variable name can be a construct
  `<name>_<index>` for an indexed variable.
*/
const jb_var* JSONBridge::find (const std::string& name, int* pidx) const
{
  int tmpidx;
  string lookup = name;

  if (!pidx)
    pidx = &tmpidx;

  size_t pnum = lookup.find_first_of ('_');
  *pidx = 0;

  if (pnum != string::npos)
  {
    string stail = lookup.substr (pnum + 1).c_str ();
    char *tail;
    *pidx = strtol (stail.c_str(), &tail, 10);

    if (*tail || *pidx < 0) //if name doesn't match <var>_<number> or index is negative...
    {                       //...search dictionary for whole name
      *pidx = 0;
    }
    else
      lookup.erase (pnum);
  }

  int lvl = 0;
  for (auto entry = dict_.data(); lvl >= 0; entry++)
  {
    //search only top level entries
    if (entry->type == JT_OBJECT)
      lvl++;
    else if (entry->type == JT_ENDOBJ)
    {
      lvl--;
      continue;
    }

    if (lvl <= 1 && lookup.c_str() == entry->name)
    {
      if (*pidx < entry->cnt)
        return entry;
      TRACE ("Out of bounds %s[%d]", lookup.c_str(), *pidx);
      return NULL;
    }
  }
  return NULL;
}

/*!
  Parse the URL-encoded body of a POST request assigning new values to all
  variables.
*/
bool JSONBridge::parse_urlencoded () const
{
  char val[1024];
  int idx;
  void *pv;

  str_pairs vars;
  parse_urlparams (client_->get_body (), vars);
  for (auto var = vars.begin (); var != vars.end (); var++)
  {
    if (!var->second.length ())
      continue;     // missing '=value' part of 'key=value' construct

    strcpy (val, var->second.c_str ());
    const jb_var *k = find (var->first.c_str (), &idx);
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

bool JSONBridge::parse_jsonencoded () const
{
  json::node rcvd;

  rcvd.read (client_->get_body ());
  if (rcvd.kind () == json::type::object)
  {
    for (auto p = rcvd.begin (); p != rcvd.end (); p++)
    {
      const jb_var* k = find (p.name ());
      if (!k)
      {
        TRACE ("Key %s not found in dictionary", p.name ().c_str ());
        continue;
      }
      if (p->kind () == json::type::object)
      {
        //TODO - support multiple level objects
        TRACE ("Multiple level objects not supported yet! %s", p.name ().c_str ());
        return false;
      }

      try {
        const json::node& n = *p;
        if (n.kind () == json::type::array)
        {
          for (int idx = 0; idx < n.size () && idx < k->cnt; idx++)
            deserialize_node (n[idx], k, idx);
        }
        else
          deserialize_node (n, k);
      }
      catch (erc&) {
        TRACE ("Error %d while processing node %s", p.name ().c_str ());
        return false;
      }
    }
    return true;
  }
  else if (rcvd.kind () == json::type::array)
  {
    /* array of name / value objects like the result of jQuery serializeArray
       https://api.jquery.com/serializearray/  */
    for (auto p=rcvd.begin(); p != rcvd.end(); ++p)
    {
      if (p->kind () == json::type::object
        && p->has ("name")
        && p->has("value"))
      {
        int idx;
        string name = (string)p->at("name");
        const jb_var* k = find (name, &idx);
        if (!k)
        {
          TRACE ("Key %s not found in dictionary", name);
          continue;
        }
        try {
          deserialize_node (p->at("value"), k, idx);
        }
        catch (erc&) {
          TRACE ("Error %d while processing node %s", name);
          return false;
        }
      }
    }
    return true;
  }
  return false; //only objects and some arrays can be parsed
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
    erc ret;
    if ((ret = ctx->json_begin (root)) == ERR_SUCCESS)
      ctx->json_end (root);
    else if (ret == ERR_JSON_NOTFOUND)
      ctx->not_found (query);
    else
      client.serve404 ();
  }
  else if (!_strcmpi (req, "POST"))
  {
    bool ok = false;
    std::string content = client.get_ihdr ("Content-Type");
    //make lower case
    std::transform (content.begin (), content.end (), content.begin(), 
      [] (char c)->char {return tolower (c); });


    const jb_var* entry;
    if (strlen (client.get_query ())
      && (entry = ctx->find (client.get_query ()))
      && entry->type == JT_POSTFUN)
    {
      uri_handler (entry->addr)(uri, client, ctx);
      ok = true;
    }
    else if (content.find("application/x-www-form-urlencoded") != string::npos)
      ok = ctx->parse_urlencoded ();
    else if (content.find ("application/json", 0) != string::npos)
      ok = ctx->parse_jsonencoded ();

    if (ok && ctx->action)
      ctx->action (*ctx);
  }
  ctx->client_ = nullptr;
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

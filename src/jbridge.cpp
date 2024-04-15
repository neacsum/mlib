/*!
  \file   jbridge.cpp Implementation of JSONBridge class

  (c) Mircea Neacsu 2017
*/
#include <mlib/mlib.h>
#pragma hdrstop
#include <memory.h>
#include <string.h>
#include <sstream>

#include <algorithm>
#include <utf8/utf8.h>

using namespace std;

namespace mlib {

#define MAX_JSONRESPONSE 8192

static bool url_decode (std::string& s);

static std::tuple<std::string, size_t> index_split (const std::string name);

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

/// Creates a JSONBridge object for the given path
JSONBridge::JSONBridge (const char* path)
  : path_ (path)
  , client_ (nullptr)
  , action (nullptr)
{}

JSONBridge::~JSONBridge ()
{}

/// Attach JSONBridge object to a HTTP server
void JSONBridge::attach_to (httpd& server)
{
  server.add_handler (path_.c_str (), (uri_handler)JSONBridge::callback, this);
}

///
erc JSONBridge::json_begin (json::node& root)
{
  TRACE9 ("JSONBridge::json_begin - %s", client ()->get_query ());
  mlib::lock l (in_use);
  size_t idx;
  dict_cptr pvar;

  if (!find (client ()->get_query (), pvar, &idx))
  {
    TRACE2 ("JSONBridge::json_begin - Cannot find %s", client ()->get_query ());
    return erc (ERR_JSON_NOTFOUND);
  }
  return jsonify (root, pvar);
}

/// Send out the JSON formatted buffer
erc JSONBridge::json_end (json::node& obj)
{
  try
  {
    // serialize in a buffer to find content length
    stringstream ss;
    ss << fixed << obj;

    client ()->out () << "HTTP/1.1 200 OK\r\n"
                         "Cache-Control: no-cache, no-store\r\n"
                         "Content-Type: application/json\r\n"
                         //"Access-Control-Allow-Origin: *\r\n"
                         "Connection: Keep-Alive\r\n"
                         "Content-Length: "
                      << ss.str ().size ()
                      << "\r\n"
                         "\r\n"
                      << ss.str () << endl;
  }
  catch (erc& x)
  {
    x.reactivate ();
    return x;
  }
  return erc::success;
}

/// Serializes a variable to JSON format
erc JSONBridge::jsonify (json::node& n, dict_cptr v)
{
  try
  {
    if (v->cnt > 1)
    {
      for (size_t i = 0; i < v->cnt; i++)
        serialize_node (n[i], v, i);
      return erc::success;
    }
    else
      return serialize_node (n, v);
  }
  catch (erc& x)
  {
    x.reactivate ();
    return x;
  }
}

erc JSONBridge::serialize_node (json::node& n, dict_cptr v, size_t index)
{
  char* addr;

  addr = (char*)v->addr + v->sz * index;

  switch (v->type)
  {
  case JT_PSTR:
    n = *(char**)addr;
    break;
  case JT_STR:
    n = *(string*)addr;
    break;
  case JT_CSTR:
    n = (char*)addr;
    break;
  case JT_SHORT:
    n = (int)*(short int*)addr;
    break;
  case JT_USHORT:
    n = (int)*(unsigned short int*)addr;
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
    // Composite object - go through dictionary
    for (auto p = v->children.begin (); p != v->children.end (); ++p)
      jsonify (n[p->name], p);
    break;

  default:
    TRACE ("Unexpected entry type %d", v->type);
    return erc (ERR_JSON_DICSTRUC, erc::error, json::errors);
  }
  return erc::success;
}

erc JSONBridge::deserialize_node (const json::node& n, dict_cptr v, size_t index) const
{
  void* pv;

  pv = (char*)(v->addr) + v->sz * index;

  try
  {
    switch (v->type)
    {
    case JT_PSTR:
      /* We don't know the max size. Treat them as read-only. */
      break;
    case JT_STR:
      *(string*)pv = n.to_str ();
      break;
    case JT_CSTR:
      strncpy ((char*)pv, (const char*)n, v->sz);
      if (v->sz)
        *((char*)pv + v->sz - 1) = 0; // always null-terminated
      break;

    case JT_INT:
      *(int*)pv = static_cast<int> (n);
      break;
    case JT_UINT:
      *(unsigned int*)pv = static_cast<int> (n);
      break;
    case JT_SHORT:
      *(short*)pv = static_cast<int> (n);
      break;
    case JT_USHORT:
      *(unsigned short*)pv = static_cast<int> (n);
      break;
    case JT_LONG:
      *(long*)pv = static_cast<int> (n);
      break;
    case JT_ULONG:
      *(unsigned long*)pv = static_cast<int> (n);
      break;
    case JT_FLT:
      *(float*)pv = (float)static_cast<double> (n);
      break;
    case JT_DBL:
      *(double*)pv = static_cast<double> (n);
      break;
    case JT_BOOL:
      *(bool*)pv = static_cast<bool> (n);
      break;
    default:
      TRACE ("Unexpected entry type: %d", v->type);
    }
  }
  catch (mlib::erc& x)
  {
    x.reactivate ();
    return x;
  }
  return erc::success;
}

void JSONBridge::not_found (const char* varname)
{
  string tmp = "HTTP/1.1 415 Unknown variable %s\r\n";
  tmp += varname;

  client ()->out () << "HTTP/1.1 415 Unknown variable " << varname << "\r\n"
                    << "Content-Type: text/plain\r\n"
                    << "Content-Length: " << tmp.size () << "\r\n\r\n"
                    << tmp << endl;
}

/*!
  Search a variable in JSON dictionary. The variable name can be a construct
  `<name>_<index>` for an indexed variable.
*/
bool JSONBridge::find (const std::string& name, dict_cptr& found, size_t* pidx) const
{
  size_t tmpidx;
  string lookup;

  if (!pidx)
    pidx = &tmpidx;

  tie (lookup, *pidx) = index_split (name);

  for (auto ptr = dict_.begin (); ptr != dict_.end (); ++ptr)
  {
    if (lookup == ptr->name)
    {
      if (*pidx < ptr->cnt)
      {
        found = ptr;
        return true;
      }
      TRACE ("Out of bounds %s[%d]", lookup.c_str (), *pidx);
      return false;
    }
  }
  return false;
}

bool JSONBridge::deep_search (const std::string& var, const dictionary& dict, dict_cptr& found)
{
  auto ptr = dict.cbegin ();
  while (ptr != dict.end ())
  {
    if (ptr->name == var)
    {
      found = ptr;
      return true;
    }
    else if (!ptr->children.empty ())
    {
      if (deep_search (var, ptr->children, found))
        return true;
    }
    ++ptr;
  }
  return false;
}

bool JSONBridge::deep_find (const std::string& name, dict_cptr& found, size_t* pidx) const
{
  size_t tmpidx;
  string lookup;

  if (!pidx)
    pidx = &tmpidx;

  tie (lookup, *pidx) = index_split (name);

  if (deep_search (lookup, dict_, found))
  {
    if (*pidx >= found->cnt)
    {
      TRACE ("Out of bounds %s[%d]", lookup.c_str (), *pidx);
      return false;
    }
    return true;
  }
  return false;
}

/*!
  Parse the URL-encoded body of a POST request assigning new values to all
  variables.
*/
bool JSONBridge::parse_urlencoded () const
{
  size_t idx;
  void* pv;

  str_pairs vars;
  parse_urlparams (client_->get_body (), vars);
  for (auto var = vars.begin (); var != vars.end (); ++var)
  {
    if (!var->second.length ())
      continue; // missing '=value' part of 'key=value' construct

    dict_cptr entry_ptr;
    string& value = var->second;

    if (!deep_find (var->first, entry_ptr, &idx))
    {
      TRACE ("Posted key %s not found in dictionary", var->first.c_str ());
      continue;
    }
    //    url_decode (value);
    pv = (char*)(entry_ptr->addr) + entry_ptr->sz * idx;

    TRACE9 ("Setting %s[%d] = %s\n", entry_ptr->name.c_str (), idx, value.c_str ());
    switch (entry_ptr->type)
    {
    case JT_PSTR:
      /* We don't know the max size. Treat them as read-only. */
      break;
    case JT_STR:
      *(string*)pv = value;
      break;
    case JT_CSTR:
      strncpy ((char*)pv, value.c_str (), entry_ptr->sz);
      *((char*)pv + entry_ptr->sz - 1) = 0; // always null-terminated
      break;
    case JT_INT:
      *(int*)pv = (int)stol (value, nullptr, 0);
      break;
    case JT_UINT:
      *(unsigned int*)pv = (unsigned int)stoul (value, nullptr, 0);
      break;
    case JT_SHORT:
      *(short*)pv = (short)stol (value, nullptr, 0);
      break;
    case JT_USHORT:
      *(unsigned short*)pv = (unsigned short)stoul (value, nullptr, 0);
      break;
    case JT_LONG:
      *(long*)pv = stol (value, nullptr, 0);
      break;
    case JT_ULONG:
      *(unsigned long*)pv = stoul (value, nullptr, 0);
      break;
    case JT_FLT:
      *(float*)pv = (float)stod (value);
      break;
    case JT_DBL:
      *(double*)pv = stod (value);
      break;
    case JT_BOOL:
      utf8::tolower (value);
      *(bool*)pv = (value == "true" || value == "1" || value == "on");
      break;

    default:
      TRACE ("Unexpected entry type: %d", entry_ptr->type);
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
      dict_cptr k;
      if (!deep_find (p.name (), k))
      {
        TRACE ("Key %s not found in dictionary", p.name ().c_str ());
        continue;
      }
      if (p->kind () == json::type::object)
      {
        // TODO - support multiple level objects
        TRACE ("Multiple level objects not supported yet! %s", p.name ().c_str ());
        return false;
      }

      try
      {
        const json::node& n = *p;
        if (n.kind () == json::type::array)
        {
          for (size_t idx = 0; idx < (size_t)n.size () && idx < k->cnt; idx++)
            deserialize_node (n[idx], k, idx);
        }
        else
          deserialize_node (n, k);
      }
      catch (erc&)
      {
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
    for (auto p = rcvd.begin (); p != rcvd.end (); ++p)
    {
      if (p->kind () == json::type::object && p->has ("name") && p->has ("value"))
      {
        size_t idx;
        string name = (string)p->at ("name");
        dict_cptr k;
        if (!deep_find (name, k, &idx))
        {
          TRACE ("Key %s not found in dictionary", name);
          continue;
        }
        try
        {
          deserialize_node (p->at ("value"), k, idx);
        }
        catch (erc&)
        {
          TRACE ("Error %d while processing node %s", name);
          return false;
        }
      }
    }
    return true;
  }
  return false; // only objects and some arrays can be parsed
}

int JSONBridge::callback (const char* uri, http_connection& client, JSONBridge* ctx)
{
  const char* req = client.get_method ();
  const char* query = client.get_query ();
  TRACE9 ("ui_callback req=%s query=%s", req, query);
  ctx->client_ = &client;
  ctx->lock ();
  if (!_strcmpi (req, "GET"))
  {
    json::node root;
    erc ret;
    if ((ret = ctx->json_begin (root)) == erc::success)
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
    // make lower case
    std::transform (content.begin (), content.end (), content.begin (),
                    [] (char c) -> char { return tolower (c); });

    if (strlen (client.get_query ()))
    {
      auto ph = ctx->post_handlers.find (client.get_query ());
      if (ph != ctx->post_handlers.end ())
      {
        (ph->second) (uri, *ctx);
        ok = true;
      }
    }
    if (!ok)
    {
      if (content.find ("application/x-www-form-urlencoded") != string::npos)
        ok = ctx->parse_urlencoded ();
      if (content.find ("application/json", 0) != string::npos)
        ok = ctx->parse_jsonencoded ();
    }

    if (ok && ctx->action)
      ctx->action (*ctx);
  }
  ctx->client_ = nullptr;
  ctx->unlock ();
  return 1;
}

/*!
  Decoding of URL-encoded data.
  We can do it in place because resulting string is shorter or equal than input.

  \return `true` if successful, `false` otherwise
*/
bool url_decode (std::string& s)
{
  size_t in = 0, out = 0;

  auto hexdigit = [] (char* bin, char c) -> bool {
    if (c >= '0' && c <= '9')
      *bin = c - '0';
    else if (c >= 'A' && c <= 'F')
      *bin = c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
      *bin = c - 'a' + 10;
    else
      return false;

    return true;
  };

  while (in < s.size ())
  {
    if (s[in] == '%')
    {
      in++;
      char d1, d2; // hex digits
      if (!hexdigit (&d1, s[in++]) || !hexdigit (&d2, s[in]))
        return false;
      s[out++] = (d1 << 4) | d2;
    }
    else if (s[in] == '+')
      s[out++] = ' ';
    else
      s[out++] = s[in];
    in++;
  }
  s.erase (out);
  return true;
}

//  If input string is of the format <var>_<number>, splits it in components
std::tuple<std::string, size_t> index_split (const std::string name)
{
  string var = name;
  int idx = 0;

  size_t pnum = var.find_first_of ('_');
  if (pnum != string::npos)
  {
    string stail = var.substr (pnum + 1).c_str ();
    char* tail;
    idx = strtol (stail.c_str (), &tail, 10);

    if (!*tail && idx >= 0)
      var.erase (pnum); // name matches <var>_<number> pattern
  }
  return {var, idx};
}

} // namespace mlib

/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This is part of MLIB project. See LICENSE file for full license terms.
*/

#include <mlib/mlib.h>
#pragma hdrstop

#include <memory.h>
#include <string.h>
#include <sstream>
#include <cassert>

#include <utils.h>
#include <utf8/utf8.h>

using namespace std;

namespace mlib::http {


static std::tuple<std::string, size_t> index_split (const std::string name);

/*!
  \class mlib::http::jbridge
  \ingroup http

  This class provides JSON serialization/deserialization of variables. It gives
  an easy access mechanism to program variables for the HTTP server.
  
  A `jbridge` object is attached to a HTTP server using the
  attach_to() function. Behind the scene, this function registers a handler
  for the path associated with the `jbridge`.

  ### Sample Usage ###
  Assume the HTTP server `hs` answers requests sent to `http://localhost:8080`
  and the `jbridge` object was created as:
  \code
    jbridge jb("/var");
  \endcode

  Then:
  \code
    jb.attach_to(hs);
  \endcode
  will register a handler for requests to `http://localhost:8080/var`

  When this handler is invoked with a GET request for 
  http://localhost:8080/var?data, it will search in the JSON dictionary a
  variable called 'data' and return its content as a JSON object.
*/

/// Creates a jbridge object for the given path
jbridge::jbridge (const char* path)
  : path_ (path)
  , client_ (nullptr)
  , post_action (nullptr)
{
  assert (path[0] == '/');
}

jbridge::~jbridge ()
{}

/// Attach jbridge object to a HTTP server
void jbridge::attach_to (server& server)
{
  server.add_handler (path_.c_str (), jbridge::callback, this);
}

///
erc jbridge::json_begin (json::node& root)
{
  TRACE9 ("jbridge::json_begin - %s", client ().get_query ());
  mlib::lock l (in_use);
  size_t idx;
  dict_cptr pvar;

  if (!find (client ().get_query (), pvar, &idx))
  {
    TRACE2 ("jbridge::json_begin - Cannot find %s", client ().get_query ().c_str());
    return erc (HTTP_JSON_NOTFOUND);
  }
  return jsonify (root, pvar);
}

/// Send out the JSON formatted buffer
erc jbridge::json_end (json::node& obj)
{
  try
  {
    // serialize in a buffer to find content length
    stringstream ss;
    ss << fixed << obj;
    client ().add_ohdr ("Cache-Control", "no-cache, no-store");
    client ().add_ohdr ("Content-Type", "application/json");
    client ().add_ohdr ("Content-Length", to_string(ss.str ().size ()));
    client().respond (200);
    client ().out () << ss.str () << endl;
  }
  catch (erc& x)
  {
    x.reactivate ();
    return x;
  }
  return erc::success;
}

/// Serializes a variable to JSON format
erc jbridge::jsonify (json::node& n, dict_cptr v)
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

erc jbridge::serialize_node (json::node& n, dict_cptr v, size_t index)
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
    return erc (HTTP_JSON_DICSTRUC, json::Errors ());
  }
  return erc::success;
}

erc jbridge::deserialize_node (const json::node& n, dict_cptr v, size_t index) const
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

/// Generate 410 response if a variable was not found in JSON dictionary
void jbridge::not_found (const char* varname)
{
  string tmp = "Unknown variable "s + varname;
  client ().add_ohdr ("Content-Type", "text/plain");
  client ().add_ohdr ("Content-Length", to_string(tmp.size ()));
  client ().respond (410, tmp.c_str ());

  client ().out () << tmp;
}

/*!
  Search a variable in JSON dictionary. The variable name can be a construct
  `<name>_<index>` for an indexed variable.
*/
bool jbridge::find (const std::string& name, dict_cptr& found, size_t* pidx) const
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

bool jbridge::deep_search (const std::string& var, const dictionary& dict, dict_cptr& found)
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

/*!
  Search for a variable name in JSON dictionary
*/
bool jbridge::deep_find (const std::string& name, dict_cptr& found, size_t* pidx) const
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
bool jbridge::parse_urlencoded () const
{
  size_t idx;
  void* pv;

  str_pairs vars;
  internal::parse_urlparams (client_->get_body (), vars);
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
      internal::str_lower (value);
      *(bool*)pv = (value == "true" || value == "1" || value == "on");
      break;

    default:
      TRACE ("Unexpected entry type: %d", entry_ptr->type);
    }
  }
  return true;
}

/// Parse a JSON-encoded body of a POST message
bool jbridge::parse_jsonencoded () const
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

void jbridge::process_request ()
{
  try
  {
    if (client ().get_method () == "GET")
    {
      json::node root;
      erc ret;
      if ((ret = json_begin (root)) == erc::success)
        json_end (root);
      else if (ret == HTTP_JSON_NOTFOUND)
        not_found (client ().get_query ().c_str ());
      else
        client ().serve404 ();
    }
    else if (client ().get_method () == "POST")
    {
      if (!client ().has_ihdr ("Content-Type"))
      {
        client ().respond (403);
        return;
      }
      std::string content = client ().get_ihdr ("Content-Type");
      internal::str_lower (content);

      bool ok = (content == "application/x-www-form-urlencoded") ? parse_urlencoded ()
              : (content == "application/json")                  ? parse_jsonencoded ()
                                                                 : false; 

      if (!ok)
      {
        client ().respond (400);
        return;
      }

      if (!client ().get_query ().empty ())
      {
        const auto& h = post_handlers.find (client ().get_query ());
        if (h != post_handlers.end ())
          ok = (h->second(client(), this) == HTTP_OK);
      }
      if (ok && post_action)
        ok = (post_action (client(), this) == HTTP_OK);

      if (ok && !redirect_uri.empty())
        client ().redirect (redirect_uri);
    }
  }
  catch (erc& x)
  {
    TRACE ("jbridge::process_request erc %d", (int)x);
    client ().respond (500); //server error
  }
}

int jbridge::callback (connection& client, void* par)
{
  jbridge* bridge = (jbridge*)par;
  auto& req = client.get_method ();
  auto& query = client.get_query ();
  TRACE9 ("ui_callback req=%s query=%s", req.c_str (), query.c_str());

  bridge->lock ();
  bridge->client_ = &client;
  bridge->process_request ();
  bridge->client_ = nullptr;
  bridge->unlock ();
  return HTTP_OK;
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

} // end namespace mlib::http

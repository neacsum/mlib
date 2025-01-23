#pragma once
/*!
  \file httpd.h Implementation of http::server and http::connection classes

  (c) Mircea Neacsu 2007-2025. All rights reserved.
*/

#include "tcpserver.h"
#include <string>
#include <map>
#include <deque>
#include <typeindex>
#include <filesystem>

/// Maximum size of HTTP header
#define HTTPD_MAX_HEADER 8192

/// \name Error codes
///\{
#define HTTPD_OK        0  ///< Success
#define HTTPD_ERR_WRITE -1 ///< Socket write failure
#define HTTPD_ERR_FOPEN -2 ///< File open failure
#define HTTPD_ERR_FREAD -3 ///< File read failure
///\}

namespace mlib::http {

class connection;

/// Case insensitive comparison function
struct ci_less : public std::function<bool (std::string, std::string)>
{
  bool operator() (const std::string& lhs, const std::string& rhs) const
  {
    return _stricmp (lhs.c_str (), rhs.c_str ()) < 0;
  }
};

/// Key-value string pairs used for headers, URL-encoded data, etc.
/// Keys are case insensitive.
typedef std::map<std::string, std::string, ci_less> str_pairs;

/// User defined URL handler function
typedef int (*uri_handler) (connection& client, void* info);

/// Representation of a HTTP client connection request
class connection : public thread
{
public:
  friend class server;

  const std::string& get_uri () const;
  const std::string& get_method () const;
  const std::string& get_query () const;
  const std::string& get_body () const;

  void add_ohdr (const std::string& hdr, const std::string& value);
  bool has_ihdr (const std::string& hdr) const;
  const std::string& get_ihdr (const std::string& hdr) const;
  bool has_ohdr (const std::string& hdr) const;
  const std::string& get_ohdr (const std::string& hdr) const;
  bool has_qparam (const std::string& key);
  const std::string& get_qparam (const std::string& key);
  bool has_bparam (const std::string& key);
  const std::string& get_bparam (const std::string& key);

  int get_content_length () const;
  sockstream& out ();
  
  void respond (unsigned int code, const char* reason = 0);
  void redirect (const std::string& uri, unsigned int code = 303);
  void serve404 (const char* text = 0);
  int serve_file (const std::filesystem::path& fname);
  int serve_shtml (const std::filesystem::path& fname);
  int serve_buffer (const BYTE* buffer, size_t sz);
  int serve_buffer (const std::string& str);

  void respond_part (const char* part_type, const char* bound);
  void respond_next (bool last);

protected:
  connection (sock& socket, server& server);
  void run () override;
  void term () override;

  server& parent;
  sockstream ws;

private:
  bool parse_request (const std::string& req);
  bool parse_headers ();
  bool parse_body ();
  void parse_query ();
  void process_valid_request ();
  void process_ssi (const char* request);
  int do_auth ();
  void serve401 (const char* realm);

  std::string uri;           ///< location
  std::string query;         ///< query string
  std::string method;        ///< query method (GET, POST, etc.)
  std::string http_version;  ///< HTTP version string
  std::string headers;       ///< all headers
  std::string body;          ///< request body
  int content_len;           ///< content length or -1 if not known
  std::string part_boundary; ///< multi-part boundary
  bool response_sent;        ///< response function has been called
  str_pairs oheaders;
  str_pairs iheaders;
  str_pairs qparams;
  str_pairs bparams;
  bool query_parsed, body_parsed;

  struct user
  {
    std::string name;
    std::string pwd;
  };
  std::multimap<std::string, user> auth; // authenticated users
};

/// Small multi-threaded HTTP server
class server : public tcpserver
{
public:
  explicit server (unsigned short port = 0, unsigned int maxconn = 0);
  ~server ();

  void add_ohdr (const std::string& hdr, const std::string& value);
  void remove_ohdr (const std::string& hdr);
  void add_handler (const std::string& uri, uri_handler func, void* info = 0);
  void add_post_handler (const std::string& uri, uri_handler func, void* info = 0);
  bool add_user (const char* realm, const char* username, const char* pwd);
  bool remove_user (const char* realm, const char* username);
  void add_realm (const char* realm, const char* uri);

  template <typename T>
  void add_var (const std::string& name, const T* addr, const char* fmt = nullptr)
  {
    constexpr vtype t = std::is_same_v<T, bool>             ? VT_BOOL
                        : std::is_same_v<T, char>           ? VT_CHAR
                        : std::is_same_v<T, std::string>    ? VT_STRING
                        : std::is_same_v<T, short>          ? VT_SHORT
                        : std::is_same_v<T, unsigned short> ? VT_USHORT
                        : std::is_same_v<T, int>            ? VT_INT
                        : std::is_same_v<T, unsigned int>   ? VT_UINT
                        : std::is_same_v<T, long>           ? VT_LONG
                        : std::is_same_v<T, unsigned long>  ? VT_ULONG
                                                            : VT_UNKNOWN;
    static_assert (t != VT_UNKNOWN);
    add_var (name, t, addr, fmt);
  }

  template <typename T>
  std::enable_if_t<std::is_floating_point_v<T>> add_var (const std::string& name, const T* addr,
                                                          const char* fmt = nullptr,
                                                          double multiplier = 1.)
  {
    constexpr vtype t = std::is_same_v<T, float>   ? VT_FLOAT
                        : std::is_same_v<T, float> ? VT_DOUBLE
                                                    : VT_UNKNOWN;
    static_assert (t != VT_UNKNOWN);
    add_var (name, t, addr, fmt, multiplier);
  }

  const std::string get_var (const std::string& name);
  void aquire_varlock ();
  void release_varlock ();
  bool try_varlock ();

  void name (const char* nam);
  void docroot (const std::string& path);
  const std::filesystem::path& docroot () const;
  void add_alias (const std::string& uri, const std::string& path);

  void default_uri (const std::string& name)
  {
    defuri = name;
  };
  const std::string& default_uri ()
  {
    return defuri;
  };

  static void add_mime_type (const std::string& ext, const std::string& type, bool shtml = false);
  static void delete_mime_type (const std::string& ext);

  bool is_protected (const std::string& uri, std::string& realm);
  bool authenticate (const std::string& realm, const std::string& user, const std::string& pwd);

  friend class connection;

protected:
  /// SSI variables type
  enum vtype
  {
    VT_UNKNOWN, ///< illegal
    VT_BOOL,    ///< bool*
    VT_CHAR,    ///< char*
    VT_STRING,  ///< std::string*
    VT_SHORT,   ///< short int*
    VT_USHORT,  ///< unsigned short*
    VT_INT,     ///< int*
    VT_UINT,    ///< unsigned int*
    VT_LONG,    ///< long*
    VT_ULONG,   ///< unsigned long*
    VT_FLOAT,   ///< float*
    VT_DOUBLE   ///< double*
  };
  int invoke_handler (connection& client);
  int invoke_post_handler (connection& client);

  bool find_alias (const std::string& res, std::filesystem::path& path);
  const std::string& guess_mimetype (const std::filesystem::path& fn, bool& shtml);
  connection* make_thread (sock& connection);
  void add_var (const std::string& name, vtype t, const void* addr, const char* fmt = nullptr,
                double multiplier = 1.);

private:
  str_pairs out_headers;          //!< response headers
  mlib::criticalsection hdr_lock; ///<! headers access lock
  str_pairs realms;               //!< access control realms

  struct handle_info
  {
    handle_info (uri_handler h_, void* info_);

    uri_handler h;
    void* nfo;
    std::shared_ptr<mlib::criticalsection> in_use;
  };
  std::map<std::string, handle_info> handlers;
  std::map<std::string, handle_info> post_handlers;

  std::map<std::string, std::string> aliases;

  struct var_info
  {
    std::string fmt;
    vtype type;
    const void* addr;
    double multiplier;
  };
  std::map<std::string, var_info> variables;
  mlib::criticalsection varlock;

  struct user
  {
    std::string name;
    std::string pwd;
  };
  std::multimap<std::string, user> credentials;

  std::filesystem::path root;
  std::string defuri;
};

/*==================== INLINE FUNCTIONS ===========================*/

/// Return URI of this connection
inline
const std::string& connection::get_uri () const
{
  return uri;
};

/// Return HTTP method (GET, POST, etc.)
inline
const std::string& connection::get_method () const
{
  return method;
};

/// Return URI query string (everything after '?' and before '#')
inline
const std::string& connection::get_query () const
{
  return query;
};

/// Return request body
inline
const std::string& connection::get_body () const
{
  return body;
};

/*!
  Add or modify a response header.

  \param  hdr   header name
  \param  value header value

  To have any effect, this function should be called before calling the
  response() (or serve_...) function as all headers are sent at that time.
*/
inline
void connection::add_ohdr (const std::string& hdr, const std::string& value)
{
  oheaders[hdr] = value;
}

/// Returns `true` if request has the header
inline
bool connection::has_ihdr (const std::string& hdr) const
{
  return iheaders.find (hdr) != iheaders.end();
}

/*!
  Returns the value of a request header.

  \param  hdr   header name
  \return header value

  Throws an exception of type `std::out_of_range` if request doesn't have that
  header
*/
inline
const std::string& connection::get_ihdr (const std::string& hdr) const
{
  return iheaders.at(hdr);
}

inline 
bool connection::has_ohdr (const std::string& hdr) const
{
  lock l (parent.hdr_lock);
  return parent.out_headers.find (hdr) != parent.out_headers.end()
    || (oheaders.find(hdr) != oheaders.end());
}

/*!
  Returns the value of a response header. The header can belong either to server
  or to connection.

  \param  hdr  header name
  \return header field value

  Throws an exception of type `std::out_of_range` if the response doesn't have
  that header.
*/
inline
const std::string& connection::get_ohdr (const std::string& hdr) const
{
  lock l (parent.hdr_lock);
  auto idx = parent.out_headers.find (hdr);
  if (idx != parent.out_headers.end ())
    return idx->second;

  return oheaders.at (hdr);
}

/// Return `true` if the query contains the given parameter
inline
bool connection::has_qparam (const std::string& key)
{
  if (!query_parsed)
    parse_query ();
  return qparams.find (key) != qparams.end ();
}

/*!
  Return the value of a query parameter

  \param key query parameter
  \return parameter value

  Throws an exception of type `std::out_of_range` if the query doesn't have
  the parameter.

  Even though query parameters and their values are URL encoded, the returned
  value is decoded.
*/
inline
const std::string& connection::get_qparam (const std::string& key)
{
  if (!query_parsed)
    parse_query ();
  return qparams.at (key);
}

/// Return true if the body contains the given parameter
inline
bool connection::has_bparam (const std::string& key)
{
  if (!body_parsed)
    parse_body ();
  return bparams.find (key) != bparams.end ();
}

/*!
  Return the value of a form parameter

  \param key form parameter
  \return parameter value

  Throws an exception of type `std::out_of_range` if the form doesn't have
  the parameter.
*/
inline
const std::string& connection::get_bparam (const std::string& key)
{
  static const std::string empty;
  if (!body_parsed)
    parse_body ();
  
  return bparams.at (key);
}

/// Return size of request body
inline
int connection::get_content_length () const
{
  return content_len;
}

/// Return socket object associated with this connection
inline
sockstream& connection::out ()
{
  return ws;
}

//------------------------ http::server inline functions ----------------------
/// Return current file origin
inline
const std::filesystem::path& server::docroot () const
{
  return root;
}

/// Acquire lock on server's variables
inline
void server::aquire_varlock ()
{
  varlock.enter ();
}

/// Release lock on server's variables
inline
void server::release_varlock ()
{
  varlock.leave ();
}

/*!
  Try to acquire lock on server's variables
  \return _true_ if lock was acquired
*/
inline
bool server::try_varlock ()
{
  return varlock.try_enter ();
}

inline
server::handle_info::handle_info (uri_handler h_, void* info_)
  : h (h_)
  , nfo (info_)
  , in_use{std::make_shared<criticalsection> ()}
{}

} // namespace mlib::http

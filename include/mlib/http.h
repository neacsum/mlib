/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file http.h Definition of mlib::http::server and mlib::http::connection classes
#pragma once

#include "tcpserver.h"
#include <string>
#include <map>
#include <deque>
#include <typeindex>
#include <filesystem>

/// Maximum size of HTTP header
#define HTTP_MAX_HEADER 8192

/// Default timeout interval while waiting for a client request
#define HTTP_TIMEOUT 30s

/// \name Error codes
///\{
#define HTTP_OK         0     ///< Success
#define HTTP_ERR_WRITE  -1    ///< Socket write failure
#define HTTP_ERR_FOPEN  -2    ///< File open failure
#define HTTP_ERR_FREAD  -3    ///< File read failure
#define HTTP_NO_HANDLER -4    ///< No handler found
#define HTTP_CONTINUE   1     ///< Continue serving page
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
typedef std::function<int (connection& client, void* info)> uri_handler;

/// Representation of a HTTP client connection request
class connection : public thread
{
  friend class server;

public:

  /// Return request target of this connection
  const std::string& get_path () const;

  /// Return HTTP method (GET, POST, etc.) of the request
  const std::string& get_method () const;

  /// Return request query string (everything after '?' and before '#')
  const std::string& get_query () const;

  /// Return request body
  const std::string& get_body () const;

  void add_ohdr (const std::string& hdr, const std::string& value);

  /// Check if request has a header
  bool has_ihdr (const std::string& hdr) const;

  /// Return the value of a request header
  const std::string& get_ihdr (const std::string& hdr) const;

  /// Return all request headers
  const str_pairs& get_request_headers () const;

  /// Return all response headers
  const str_pairs& get_response_headers () const;

  /// Check if response has a header
  bool has_ohdr (const std::string& hdr) const;

  /// Return the value of a response header
  const std::string& get_ohdr (const std::string& hdr) const;

  /// Check if query has a parameter
  bool has_qparam (const std::string& key);

  /// Return the value of a query parameter
  const std::string& get_qparam (const std::string& key);

  /// Return true if request contains the given parameter in the request body
  bool has_bparam (const std::string& key);

  /// Return the value of a body parameter
  const std::string& get_bparam (const std::string& key);

  /// Return authenticated user name
  const std::string& get_auth_user ();

  /// Return size of request body
  int get_content_length () const;

  sockstream& out ();
  
  void respond (unsigned int code, const std::string& reason = std::string());
  void redirect (const std::string& uri, unsigned int code = 303);
  void serve404 (const char* text = 0);
  int serve_file (const std::filesystem::path& file);
  int serve_shtml (const std::filesystem::path& file);
  int serve_buffer (const BYTE* buffer, size_t sz);
  int serve_buffer (const std::string& str);

protected:
  connection (sock& socket, server& server);

  /// The thread run loop
  void run () override;

  /// Cleanup function invoked when connection thread terminates
  void term () override;

  /// HTTP server that created this connection
  server& parent;

  /// socket stream used for send/receive operations
  sockstream ws;

private:
  bool parse_request (const std::string& req);
  bool parse_headers (const std::string& hdrs);
  bool parse_body ();
  void parse_query ();
  void process_valid_request ();
  void process_ssi (const char* request);
  bool do_auth ();
  void serve401 (const std::string& realm);
  bool should_close ();
  void request_init ();
  void serve_options ();


  std::string path_;         ///< location
  std::string query_;        ///< query string
  std::string method_;       ///< query method (GET, POST, etc.)
  std::string http_version;  ///< HTTP version string
  std::string body;          ///< request body
  int content_len;           ///< content length or -1 if not known
  std::string part_boundary; ///< multi-part boundary
  bool response_sent;        ///< response function has been called
  str_pairs oheaders;
  str_pairs iheaders;
  str_pairs qparams;
  str_pairs bparams;
  bool query_parsed, body_parsed;
  std::string auth_user;  ///< authenticated user
  std::string auth_realm; ///< protection realm

  friend struct UnitTest_connection;
};

/// Small multi-threaded HTTP server
class server : public tcpserver
{
public:
  explicit server (unsigned short port = 0, unsigned int maxconn = 0);
  ~server ();

  void add_ohdr (const std::string& hdr, const std::string& value);
  void remove_ohdr (const std::string& hdr);

  ///  Add or modify an URI handler function
  void add_handler (const std::string& uri, uri_handler func, void* info = 0);

  ///  Add or modify an POST handler function.
  void add_post_handler (const std::string& uri, uri_handler func, void* info = 0);

  ///  Add a new user to a protection realm.
  void add_user (const std::string& realm, const std::string& username, const std::string& pwd);

  /// Remove an allowed user from an protection realm
  void remove_user (const std::string& realm, const std::string& username);

  /// Add an URI to a protection realm
  void add_secured_path (const std::string& realm, const std::string& uri);

  /// Add a variable to the dictionary of SSI variables
  ///   \tparam T variable's type (non-floating point)
  ///   \param name variable's SSI name
  ///   \param addr pointer to variable
  ///   \param fmt sprintf formatting specifier
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

  /// Add a variable to the dictionary of SSI variables
  ///   \tparam T variable's type (floating point)
  ///   \param name variable's SSI name
  ///   \param addr pointer to variable
  ///   \param fmt sprintf formatting specifier
  ///   \param multiplier scaling factor
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

  void name (const std::string& name_);
  void docroot (const std::filesystem::path& path);
  const std::filesystem::path& docroot () const;
  void add_alias (const std::string& uri, const std::string& path);

  void default_uri (const std::string& name);
  const std::string& default_uri () const;

  void keep_alive (std::chrono::seconds secs);
  std::chrono::seconds keep_alive () const;

  static void add_mime_type (const std::string& ext, const std::string& type, bool shtml = false);
  static void delete_mime_type (const std::string& ext);

  bool is_protected (const std::string& uri, std::string& realm);
  virtual bool verify_authorization (const std::string& realm, const std::string& user,
                                     const std::string& password);
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
  virtual bool locate_resource (const std::string& res, std::filesystem::path& path);

  const std::string& guess_mimetype (const std::filesystem::path& fn, bool& shtml);
  connection* make_thread (sock& connection);
  void add_var (const std::string& name, vtype t, const void* addr, const char* fmt = nullptr,
                double multiplier = 1.);
private:

  str_pairs out_headers;          //!< response headers
  mlib::criticalsection hdr_lock; ///<! headers access lock

  struct handle_info
  {
    handle_info (uri_handler h_, void* info_);

    uri_handler h;
    void* nfo;
    std::shared_ptr<mlib::criticalsection> in_use;
  };

  bool locate_handler (const std::string& res, handle_info** ptr);

  std::map<std::string, handle_info> handlers;
  std::map<std::string, handle_info> post_handlers;

  std::map<std::string, std::string> aliases;

  /// Descriptor for a SSI variable
  struct var_info
  {
    std::string fmt;
    vtype type;
    const void* addr;
    double multiplier;
  };

  /// SSI variables
  std::map<std::string, var_info> variables;
  mlib::criticalsection varlock;

  /// User authentication info
  struct user_inf
  {
    std::string name;
    std::string pwd;
  };

  /// Protection realm descriptor
  struct realm_descr
  {
    std::vector<std::string> paths; ///< protected URIs under a realm
    std::vector<user_inf> credentials;  ///< Users allowed access to realm
  };

  /// Protection realms
  std::map<std::string, realm_descr> realms; 
  mlib::criticalsection realmlock;

  std::filesystem::path root;
  std::string defuri;
  std::chrono::seconds timeout;

  friend class connection;
};

/*==================== INLINE FUNCTIONS ===========================*/

/*!
  Returns the `<target path>` component of the request line.

  The general structure of a HTTP request line is:
  ```
    <request> :=  <method> ' ' <target> ' ' <protocol version>
    <target> := <target path>['?' <query> ['#' <fragment>]]
  ```
  
  Only `origin-form` (see [RFC9112](https://www.rfc-editor.org/rfc/rfc9112#name-origin-form))
  is accepted.
*/
inline
const std::string& connection::get_path () const
{
  return path_;
};

/*!
  Returns the `<method>` component of the request line.

  The general structure of a HTTP request line is:
  ```
    <request> :=  <method> ' ' <target> ' ' <protocol version>
    <target> := <target path>['?' <query> ['#' <fragment>]]
  ```
*/
inline
const std::string& connection::get_method () const
{
  return method_;
};

/*!
  Returns the `query` component from the request line.
  
  The general structure of a HTTP request line is:
  ```
    <request> :=  <method> ' ' <target> ' ' <protocol version>
    <target> := <target path>['?' <query> ['#' <fragment>]]
  ```

  \note The returned string is not decoded.

  \return query component of the request line or empty string if request line
    doesn't include a query.
*/
inline
const std::string& connection::get_query () const
{
  return query_;
};

/*!
  \return request body or empty string if request doesn't have a body
*/
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
  respond() (or serve_...) function as response headers are sent at that time.
*/
inline
void connection::add_ohdr (const std::string& hdr, const std::string& value)
{
  oheaders[hdr] = value;
}

/// Return `true` if request has the header
inline
bool connection::has_ihdr (const std::string& hdr) const
{
  return iheaders.find (hdr) != iheaders.end();
}

/*!
  \param  hdr   header name
  \return header value

  Throws an exception of type `std::out_of_range` if request doesn't have the
  header
*/
inline
const std::string& connection::get_ihdr (const std::string& hdr) const
{
  return iheaders.at(hdr);
}

/*!
  Return `true' if connection has the response header
*/
inline 
bool connection::has_ohdr (const std::string& hdr) const
{
  return oheaders.find(hdr) != oheaders.end();
}

/*!
  \param  hdr  header name
  \return header field value

  Throws an exception of type `std::out_of_range` if response header doesn't 
  exist.
*/
inline
const std::string& connection::get_ohdr (const std::string& hdr) const
{
  return oheaders.at (hdr);
}

/*!
  Return `true` if the query contains the given parameter.

  Query parameters and their values are URL-decoded before being processed.
*/
inline
bool connection::has_qparam (const std::string& key)
{
  if (!query_parsed)
    parse_query ();
  return qparams.find (key) != qparams.end ();
}

/*!

  \param key query parameter
  \return parameter value

  Throws an exception of type `std::out_of_range` if the query doesn't have
  the parameter.

  Query parameters and their values are URL-decoded before being processed.
*/
inline
const std::string& connection::get_qparam (const std::string& key)
{
  if (!query_parsed)
    parse_query ();
  return qparams.at (key);
}

/*!
  Return `true` if request body contains the parameter.

  Request body is URL-decoded before being processed.
*/
inline
bool connection::has_bparam (const std::string& key)
{
  if (!body_parsed)
    parse_body ();
  return bparams.find (key) != bparams.end ();
}

/*!
  Only requests with content in URL-encoded format can be parsed.

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

inline const std::string& connection::get_auth_user ()
{
  return auth_user;
}

/*!
  If the request is a POST or PUT request without a "Content-Length" header,
  the request is rejected with a response code 400
*/
inline
int connection::get_content_length () const
{
  return content_len;
}

/// Return socket stream object associated with this connection
inline
sockstream& connection::out ()
{
  return ws;
}

inline
const str_pairs& connection::get_request_headers () const
{
  return iheaders;
}

inline const str_pairs& connection::get_response_headers () const
{
  return oheaders;
}


//------------------------ http::server inline functions ----------------------

/// Return current root path as absolute path
inline
const std::filesystem::path& server::docroot () const
{
  return root;
}

/// Set server root path
inline
void server::docroot (const std::filesystem::path& path)
{
  root = std::filesystem::absolute (path);
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

/// Set default file name (initially `index.html`)
inline
void server::default_uri (const std::string& name)
{
  defuri = name;
}

/// Return default file name (initially `index.html`)
inline
const std::string& server::default_uri () const
{
  return defuri;
}

/// Set timeout value for keep-alive connections
inline 
void server::keep_alive (std::chrono::seconds secs)
{
  timeout = secs;
}

/// Return timeout value for keep-alive connections
inline
std::chrono::seconds server::keep_alive () const
{
  return timeout;
}

inline
server::handle_info::handle_info (uri_handler h_, void* info_)
  : h (h_)
  , nfo (info_)
  , in_use{std::make_shared<criticalsection> ()}
{}

/*!
   Stream out a headers map as a sequence of lines
```
   <key>: <value><CR><LF>
```
*/
inline std::ostream& operator<< (std::ostream& os, const str_pairs& hdrs)
{
  for (auto& hdr : hdrs)
  {
    os << hdr.first << ": " << hdr.second << "\r\n";
  }
  return os;
}

} // namespace mlib::http

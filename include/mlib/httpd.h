#pragma once
/*!
  \file HTTPD.H - Implementation of httpd and http_connection classes

  (c) Mircea Neacsu 2007-2014. All rights reserved.
*/

#include "tcpserv.h"
#include <string>
#include <map>
#include <deque>

/// Maximum size of HTTP header
#define HTTPD_MAX_HEADER      8192

/// Default listening port for HTTP server
#define HTTPD_DEFAULT_PORT    80

/// \name Error codes
///\{
#define HTTPD_OK              0      ///< Success
#define HTTPD_ERR_WRITE       -1      ///< Socket write failure
#define HTTPD_ERR_FOPEN       -2      ///< File open failure
#define HTTPD_ERR_FREAD       -3      ///< File read failure
///\}

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

class http_connection;

///Case insensitive comparison function
struct ciLess : public std::binary_function<std::string, std::string, bool> {
  bool operator()(const std::string &lhs, const std::string &rhs) const {
    return _stricmp (lhs.c_str (), rhs.c_str ()) < 0;
  }
};

/// Key-value string pairs used for headers, URL-encoded data, etc.
/// Keys are case insensitive.
typedef std::map<std::string, std::string, ciLess> str_pairs;

/// User defined URL handler function
typedef int (*uri_handler)(const char *uri, http_connection& client, void *info);

///Representation of a HTTP client connection request
class http_connection : public thread
{
public:
  friend class httpd;

  const char* get_uri();
  const char* get_method ();
  const char* get_all_ihdr ();
  void        add_ohdr (const char *hdr, const char *value);
  const char* get_ihdr (const char *hdr);
  const char* get_ohdr (const char *hdr);
  const char* get_query ();
  const char* get_fragment ();
  const char* get_body ();
  int         get_content_length ();

  sockstream& out ();
  void        respond (unsigned int code, const char *reason=0);
  void        redirect (const char *uri, unsigned int code=303);
  void        serve404 (const char *text = 0);
  int         serve_file (const std::string& full_path);
  int         serve_buffer (BYTE *full_path, size_t sz);
  int         serve_shtml (const std::string& full_path);
  bool        parse_formbody (str_pairs& params);

  void        respond_part (const char *part_type, const char *bound);
  void        respond_next (bool last);

protected:
              http_connection (sock& socket, httpd& server);
  void        run ();
  bool        term ();

  httpd&      parent;
  sockstream  ws;

private:
  bool        parse_url ();
  bool        parse_headers ();
  void        process_valid_request ();
  void        process_ssi (const char *request);
  int         do_auth ();
  void        serve401 (const char *realm);

  char request[HTTPD_MAX_HEADER];
  char *uri;            //!< location
  char *query;          //!< query string
  char *fragment;       //!< fragment identifier
  char *http_version;   //!< HTTP version string
  char *headers;        //!< all headers
  char *body;           //!< request body
  int  content_len;     //!< content length or -1 if not known
  std::string part_boundary; //!< multi-part boundary
  bool response_sent;   //!< response function has been called
  size_t req_len;
  str_pairs oheaders;
  str_pairs iheaders;

  struct user {
    std::string name;
    std::string pwd;
  };
  std::multimap <std::string, user> auth; //authenticated users

};

/// Small multi-threaded HTTP server
class httpd : public tcpserver
{
public:
  httpd       (unsigned short port=HTTPD_DEFAULT_PORT, unsigned int maxconn=0);
  ~httpd      ();

  void        add_ohdr (const char *field, const char *value);
  void        remove_ohdr (const char *field);
  void        add_handler (const char *uri, uri_handler func, void *info);
  void        add_alias (const char *uri, const char *path);
  bool        add_user (const char *realm, const char *username, const char *pwd);
  bool        remove_user(const char *realm, const char *username);
  void        add_realm (const char *realm, const char *uri);
  void        add_var (const char *name, const char *fmt, void *addr, double multiplier=0.);
  std::string get_var (const char *name);

  void        docroot (const char *path);
  const char* docroot () {return root.c_str();};

  void        default_uri (const char *name) {defuri = name;};
  const char* default_uri () {return defuri.c_str();};

  unsigned short port ();
  void        port (unsigned short portnum);
  void        server_name (const char *name);
  const char* server_name () {return servname.c_str();};

  void        add_mime_type (const char *ext, const char *type, bool shtml=false);
  void        delete_mime_type (const char *ext);

  bool        is_protected (const char *uri, std::string &realm);
  bool        authenticate (const char *realm, const char *user, const char *pwd);

  friend class http_connection;

protected:
  int         invoke_handler (const char *uri, http_connection& client);
  bool        find_alias (const char *uri, char *path);
  const char* guess_mimetype (const char *file, bool& shtml);
  http_connection* make_thread(sock& connection);

  bool        init ();

private:
  unsigned short  port_num;       //!< port number
  str_pairs       out_headers;    //!< response headers
  str_pairs       realms;         //!< access control realms

  struct handle_info {
    uri_handler h;
    void *nfo;
  };
  std::map <std::string, handle_info> handlers;

  std::map <std::string, std::string> aliases;

  struct var_info {
    std::string fmt;
    void *addr;
    double multiplier;
  };
  std::map <std::string, var_info> variables;

  struct user {
    std::string name;
    std::string pwd;
  };
  std::multimap <std::string, user> credentials;

  struct mimetype {
    std::string suffix;
    std::string type;
    bool shtml;
  };
  std::deque <mimetype> types;

  std::string root;
  std::string defuri;
  std::string servname;
};

/*==================== INLINE FUNCTIONS ===========================*/

/// Return URI of this connection
inline
const char* http_connection::get_uri () { return uri; };

/// Return HTTP method (GET, POST, etc.)
inline
const char* http_connection::get_method () { return request; };

/// Return all incoming headers
inline
const char* http_connection::get_all_ihdr () { return headers; };

/// Return URI query string (everything after '?' and before '#')
inline
const char* http_connection::get_query () { return query?query : ""; };

/// Return URI fragment identifier (everything after '#')
inline
const char* http_connection::get_fragment () { return fragment ? fragment : ""; };

/// Return request body
inline
const char* http_connection::get_body () { return body; };

/// Return request body
inline
int http_connection::get_content_length () { return content_len; };

/// Return socket object associated with this connection
inline
sockstream& http_connection::out () { return ws; };

/// Return port number where server is listening
inline
unsigned short httpd::port () { return port_num; }


#ifdef MLIBSPACE
}
#endif

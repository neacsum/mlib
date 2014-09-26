#pragma once
/*!
  \file HTTPD.H - Implementation of httpd and http_connection classes

  (c) Mircea Neacsu 2007-2014. All rights reserved.
*/

#include "tcpserv.h"
#include <string>
#include <map>
#include <deque>

#define HTTPD_MAX_HEADER  8192
#define HTTPD_TIMEOUT     30

///Error codes
#define HTTPD_OK           0      ///< Success
#define HTTPD_ERR_WRITE   -1      ///< Socket write failure
#define HTTPD_ERR_FOPEN   -2      ///< File open failure
#define HTTPD_ERR_FREAD   -3      ///< File read failure


#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

class http_connection;

typedef std::map<std::string, std::string> pairs;
typedef int (*handler)(const char *uri, http_connection& client, void *info);

class http_connection : public thread
{
public:
  friend class httpd;

  const char* get_uri() {return uri;};
  const char* get_method () {return request;};
  const char* get_all_ihdr () {return headers;};
  void        add_ohdr (const char *hdr, const char *value);
  const char* get_ihdr (const char *hdr);
  const char* get_ohdr (const char *hdr);

  sockstream& out () {return ws;};
  const char* get_params () {return param;};
  const char* get_body () {return body;};
  void        respond (unsigned int code);
  void        redirect (const char *uri);
  void        serve404 (const char *text = 0);
  int         serve_file (const char *full_path);
  int         serve_buffer(BYTE *full_path, size_t sz);
  int         serve_shtml (const char *full_path);
  bool        parse_formbody (pairs& params);

  void        respond_part (const char *part_type, const char *bound);
  void        respond_next (bool last);

protected:
              http_connection (sock& socket, httpd& server);
  void        run ();

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
  char *param;          //!< parameters
  char *headers;        //!< all headers
  char *body;           //!< request body
  std::string part_boundary; //!< multi-part boundary
  bool response_sent;   //!< response function has been called
  size_t req_len;
  pairs oheaders;
  pairs iheaders;

  struct user {
    std::string name;
    std::string pwd;
  };
  std::multimap <std::string, user> auth; //authenticated users

};


class httpd : public tcpserver
{
public:
  httpd       (unsigned short port, unsigned int maxconn=0);
  ~httpd      ();

  void        add_ohdr (const char *field, const char *value);
  void        remove_ohdr (const char *field);
  void        add_handler (const char *uri, handler func, void *info);
  void        add_alias (const char *uri, const char *path);
  bool        add_user (const char *realm, const char *username, const char *pwd);
  bool        remove_user(const char *realm, const char *username);
  void        add_realm (const char *realm, const char *uri);
  void        add_var (const char *name, const char *fmt, void *addr, double multiplier=0.);
  std::string get_var (const char *name);

  void        docroot (const char *path) {root = path;};
  const char* docroot () {return root.c_str();};

  void        default_file (const char *name) {defname = name;};
  const char* default_file () {return defname.c_str();};

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

private:
  unsigned short  port_num;       //!< port number
  pairs           out_headers;    //!< response headers
  pairs           realms;         //!< access control realms

  struct handle_info {
    handler h;
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
  std::string defname;
  std::string servname;
};

#ifdef MLIBSPACE
}
#endif

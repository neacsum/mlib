/*!
  \file httpd.cpp Implementation of httpd and http_connection classes

  (c) Mircea Neacsu 2007-2017. All rights reserved.
*/
#include <mlib/mlib.h>
#pragma hdrstop

#if __has_include(<utf8/utf8.h>)
#define MLIB_HAS_UTF8_LIB
#include <utf8/utf8.h>
#else
#include <io.h> //for _access
#endif

using namespace std;
using namespace mlib;

// Defaults
#define HTTPD_DEFAULT_URI "index.html"     //!< Default URL
#define HTTPD_SERVER_NAME "MNCS_HTTPD 1.0" //!< Default server name

/// Timeout interval while waiting for a client response
#define HTTPD_TIMEOUT 30

/// Max length of a form parameter
#define MAX_PAR 1024

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

namespace mlib {

/// Table of known mime types
static struct smime
{
  const char* suffix;
  const char* type;
  bool shtml;
} knowntypes[] = {{"txt", "text/plain", false},
                  {"htm", "text/html", false},
                  {"html", "text/html", false},
                  {"shtml", "text/html", true},
                  {"shtm", "text/html", true},
                  {"css", "text/css", false},
                  {"xml", "text/xml", false},
                  {"json", "text/json", false},
                  {"gif", "image/gif", false},
                  {"jpg", "image/jpeg", false},
                  {"jpeg", "image/jpeg", false},
                  {"png", "image/png", false},
                  {"ico", "image/png", false},
                  {"svg", "image/svg+xml", false},
                  {"bmp", "image/bmp", false},
                  {"pdf", "application/pdf", false},
                  {"xslt", "application/xml", false},
                  {"js", "application/x-javascript", false},
                  {0, 0, false}};

static bool file_exists (char* path);
static int fmt2type (const char* fmt);

// TODO - replace with string versions. Make visible and add tests
static int url_decode (char* buf);
static int match (const char* str1, const char* str2);
static int hexbyte (char* bin, const char* str);
static int hexdigit (char* bin, char c);

//-----------------------------------------------------------------------------
/*!
  \class http_connection
  \ingroup  sockets

  This is the thread created by the httpd server object in response to a new
  connection request.

  After construction, the thread is started automatically by the server and it
  begins listening to its connection socket and process HTTP requests. When the
  HTTP client closes the connection the thread is stopped and destructed.

  Users can create derived classes but the parent server must also be a class
  derived from httpd with an overridden httpd::make_thread function.

  The request processing cycle starts with the receiving a client request,
  validating, processing it and sending back the reply. The processing part
  implies either calling a user handler function if one was registered for the
  URI or serving a file.

  Most of the public functions are designed for the benefit of user handler
  functions. The handler function is called before sending any type of reply to
  the HTTP client.
*/

/*!
  Protected constructor used by httpd class to create a new connection thread
  \param  socket  connecting socket
  \param  server  parent server object

  As it has only a protected constructor it is not possible for users of this
  class to directly create this object. Derived classes should maintain this
  convention.
*/
http_connection::http_connection (sock& socket, httpd& server)
  : thread ("http_connection")
  , parent (server)
  , ws (socket)
  , uri (0)
  , headers (0)
  , req_len (0)
  , body (0)
  , query (0)
  , http_version (0)
  , response_sent (false)
  , content_len (-1)
  , query_parsed (false)
  , body_parsed (false)
{}

/*!
  Thread run loop.

  The loop collects the client request, parses, validates and processes it.
*/
void http_connection::run ()
{
  ws->recvtimeout (HTTPD_TIMEOUT);
#if defined(MLIB_TRACE)
  inaddr addr;
  ws->peer (addr);
  TRACE8 ("http_connection::run - Connection from %s", addr.hostname ().c_str ());
#endif
  try
  {
    while (1)
    {
      bool req_ok = false;
      req_len = 0;
      content_len = -1;
      char* ptr = request;
      bool eol_seen = false;
      iheaders.clear ();
      oheaders.clear ();
      qparams.clear ();
      bparams.clear ();
      query_parsed = body_parsed = false;
      delete body;
      body = 0;

      // Accumulate HTTP request
      while (req_len < HTTPD_MAX_HEADER)
      {
        // wait for chars.
        *ptr = ws.get ();

        if (ws.eof ())
          return; // client closed

        if (!ws.good ())
        {
#if defined(MLIB_TRACE)
          inaddr addr;
          ws->peer (addr);
          TRACE2 ("http_connection::run - timeout peer %s", addr.hostname ().c_str ());
#endif
          respond (408);
          return;
        }

        if (*ptr == '\r')
          continue;
        req_len++;
        if (*ptr == '\n')
        {
          if (eol_seen)
          {
            req_ok = true;
            break;
          }
          else
            eol_seen = true;
        }
        else
          eol_seen = false;
        ptr++;
      }

      if (req_len == HTTPD_MAX_HEADER)
      {
#if defined(MLIB_TRACE)
        inaddr addr;
        ws->peer (addr);
        TRACE2 ("http_connection::run - Request too long (%d) peer %s", req_len,
                addr.hostname ().c_str ());
#endif
        respond (413);
        return;
      }

      *ptr = 0; // NULL terminated

      response_sent = false;
      if (req_ok && parse_url () && parse_headers ())
      {
        int auth_stat = do_auth ();
        if (auth_stat > 0)
        {
          if (!strcmpi (get_method (), "POST") || !strcmpi (get_method (), "PUT"))
          {
            // read request body
            const char* cl = get_ihdr ("Content-Length");
            if (cl)
              content_len = atoi (cl);
            if (content_len > 0)
            {
              body = new char[content_len + 1];
              ws.read (body, content_len);
              body[content_len] = 0;
            }
          }
          process_valid_request ();
        }
        else if (auth_stat < 0)
          return;
      }
      else
      {
        respond (400); // malformed request
        return;
      }

      // should close connection?
      const char* ph;
      if (!get_ohdr ("Content-Length") // could not generate content length (shtml pages)
          || ((ph = get_ihdr ("Connection")) && !strcmpi (ph, "Close"))  // client wants to close
          || ((ph = get_ohdr ("Connection")) && !strcmpi (ph, "Close"))) // server wants to close
        return;
      ws.flush ();
    }
  }
  catch (erc& err)
  {
    respond (500);
    TRACE ("http_connection errcode=%d\n", err.code ());
  }

  // all cleanup is done by term function
}

void http_connection::term ()
{
#if defined(MLIB_TRACE)
  inaddr addr;
  ws->peer (addr);
  TRACE8 ("http_connection::term - Closed connection to %s", addr.hostname ().c_str ());
#endif
  parent.close_connection (*ws.rdbuf ());
  thread::term ();
}

/*
  Check if uri is covered by a server realm and if user has credentials for
  said realm.

  \return
    0   = Missing authorization (401)
   -1   = Bad authorization (501)
    1   = all good

*/
int http_connection::do_auth ()
{
  string realm;
  if (!parent.is_protected (uri, realm))
    return 1;

  auto ah = iheaders.find ("Authorization");
  if (ah == iheaders.end ())
  {
    serve401 (realm.c_str ());
    return 0;
  }

  const char* ptr = ah->second.c_str ();
  if (strnicmp (ptr, "Basic ", 6))
  {
    // Other auth methods
    TRACE8 ("Authorization: %s", ptr);
    respond (501); // not implemented
    return -1;
  }

  ptr += 6;
  char buf[256];
  size_t outsz = base64dec (ptr, buf);
  buf[outsz] = 0;
  char* pwd = strchr (buf, ':');
  if (pwd)
    *pwd++ = 0;
  else
    pwd = buf;
  if (!parent.authenticate (realm.c_str (), buf, pwd))
  {
    // invalid credential
    TRACE2 ("http_connection: Invalid credentials user %s", buf);
    serve401 (realm.c_str ());
    return 0;
  }
  TRACE8 ("http_connection: Authenticated user %s for realm %s", buf, realm.c_str ());
  return 1;
}

void http_connection::serve401 (const char* realm)
{
  static const char* std401 =
    "<HTML><HEAD><TITLE>401 Unauthorized</TITLE></HEAD>"
    "<BODY><H4>401 Unauthorized!</H4>Authorization required.</BODY></HTML>";
  char len[10];
  char challenge[256];
  strcpy (challenge, "Basic realm=\"");
  strcat (challenge, realm);
  strcat (challenge, "\"");

  add_ohdr ("WWW-Authenticate", challenge);

  sprintf (len, "%d", (int)strlen (std401));
  add_ohdr ("Content-Length", len);
  add_ohdr ("Content-Type", "text/html");
  respond (401);
  ws << std401;
}

/*!
  Called after request validation
*/
void http_connection::process_valid_request ()
{
  // check first if there is any handler registered for this uri
  if (parent.invoke_handler (uri, *this))
  {
    TRACE9 ("handler done");
    return;
  }

  // set timeout to whatever client wants
  const char* ka;
  if (ka = get_ihdr ("Keep-Alive"))
  {
    int kaval = min (atoi (ka), 600);
    ws->recvtimeout (kaval);
  }

  // try to see if we can serve a file
  char fullpath[_MAX_PATH];
  if (!parent.find_alias (uri, fullpath))
  {
    strcpy (fullpath, parent.docroot ());
    strcat (fullpath, uri);
  }

  if (fullpath[strlen (fullpath) - 1] == '/' || fullpath[strlen (fullpath) - 1] == '\\')
    strcat (fullpath, parent.default_uri ());
  if (file_exists (fullpath))
  {
    bool shtml = false;
    int ret;
    if (!get_ohdr ("Content-Type"))
      add_ohdr ("Content-Type", parent.guess_mimetype (fullpath, shtml));
    if (shtml)
      ret = serve_shtml (fullpath);
    else
      ret = serve_file (fullpath);
    if (ret == -2)
      respond (403);
    TRACE9 ("served %s result = %d", fullpath, ret);
  }
  else
  {
    TRACE2 ("not found %s", fullpath);
    serve404 ();
  }
}

/*!
  Sends a 404 (page not found) response
*/
void http_connection::serve404 (const char* text)
{
  static const char* std404 = "<html><head><title>Page not found</title></head>"
                              "<body><h1>Oops! 404 - File not found</h1>"
                              "<p>The page you requested was not found.</body></html>";

  char len[10];
  if (!text)
    text = std404;

  sprintf (len, "%d", (int)strlen (text));
  add_ohdr ("Content-Length", len);
  add_ohdr ("Content-Type", "text/html");
  respond (404);
  ws << text;
}

bool http_connection::parse_headers ()
{
  char* ptr = headers;
  char *val, *next;
  while (ptr && *ptr)
  {
    if (val = strchr (ptr, ':'))
    {
      *val++ = 0;
      while (*val == ' ')
        val++;
      if (next = strchr (val, '\n'))
        *next++ = 0;
      string key (ptr);
      string value;
      if (iheaders.find (key) != iheaders.end ())
        value = iheaders[key] + ',' + string (val);
      else
        value = string (val);
      iheaders[key] = value;
      ptr = next;
    }
    else
      return false;
  }
  return true;
}

bool http_connection::parse_body ()
{
  // check if we have what to parse
  if (!body)
    return false;

  if (!strcmpi (get_ihdr ("Content-Type"), "application/x-www-form-urlencoded"))
  {
    if (body_parsed)
      return true;
    parse_urlparams (body, bparams);
    body_parsed = true;
    return true;
  }
  // TODO - add parsing of other content types
  return false;
}

/// Parse an URL encoded query
void http_connection::parse_query ()
{
  if (query_parsed)
    return; // don't repeat parsing
  parse_urlparams (query, qparams);
  query_parsed = true;
}

/*!
  Return the value of a query parameter or the empty string if the query doesn't
  have the parameter.

  \param key query parameter to look for
  \return parameter value or empty string if parameter is not found.

  Even though query parameters and their values are URL encoded, the returned
  value is decoded.

  The function also returns an empty string if the parameter exists but it
  doesn't have a value. For instance `get_qparam("mypar")` will return an empty
  string for any of these queries: `http://example.com/page/` and
  `http://examplre.com/page?mypar`. You can use the function
  http_connection::has_qparam() to distinguish between the two cases.
*/
const std::string& http_connection::get_qparam (const char* key)
{
  static const std::string empty;
  if (!query_parsed)
    parse_query ();
  auto p = qparams.find (key);
  if (p != qparams.end ())
    return p->second;

  return empty;
}

/// Return true if the query contains the given parameter
bool http_connection::has_qparam (const char* key)
{
  if (!query_parsed)
    parse_query ();
  auto p = qparams.find (key);
  return p != qparams.end ();
}

/*!
  Return the value of a form parameter or the empty string if the body doesn't
  have the parameter.
*/
const std::string& http_connection::get_bparam (const char* key)
{
  static const std::string empty;
  if (!body_parsed)
    parse_body ();
  auto p = bparams.find (key);
  if (p != bparams.end ())
    return p->second;

  return empty;
}

/// Return true if the body contains the given parameter
bool http_connection::has_bparam (const char* key)
{
  if (!body_parsed)
    parse_body ();
  auto p = bparams.find (key);
  return p != bparams.end ();
}

/*!
  Send the content of a file, processing any SSI directives.

  \param  full_path fully qualified file name (UTF8 encoded)

  \return 0 if successful or one of the following values:
  \retval -1 socket write failure
  \retval -2 file open error

  As we don't know the size of the response, the file is sent without a
  "Content-Length" header.
*/
int http_connection::serve_shtml (const std::string& full_path)
{
#ifdef MLIB_HAS_UTF8_LIB
  FILE* fn = utf8::fopen (full_path, "rS");
#else
  FILE* fn = fopen (full_path.c_str (), "rS");
#endif
  if (fn == NULL)
    return -2;

  respond (200);
  if (!strcmpi (get_method (), "HEAD"))
  {
    fclose (fn);
    return 0; // don't send body in response to a HEAD request
  }
  int parser_state = 0;
  char ssi_buf[256];
  int ssi_len;
  int ssi_seq = 0;
  static char ssi_intro[] = "<!--#";
  static char ssi_end[] = "-->";
  while (!feof (fn))
  {
    char c = fgetc (fn);
    if (feof (fn))
      break;
    if (!ws.good ())
      return -1;
    switch (parser_state)
    {
    case 0: // search for SSI intro sequence
      if (c == ssi_intro[ssi_seq])
      {
        ssi_seq++;
        if (!ssi_intro[ssi_seq])
        {
          parser_state = 1;
          ssi_seq = 0; // reuse counter for end sequence
          ssi_len = 0;
        }
      }
      else
      {
        // dump any accumulated sequence
        for (int i = 0; i < ssi_seq; i++)
          ws << ssi_intro[i];
        ssi_seq = 0;
        ws << c;
      }
      break;

    case 1: // accumulate chars between SSI markers
      if (c == ssi_end[ssi_seq])
      {
        // may reach end of request
        ssi_seq++;
        if (!ssi_end[ssi_seq])
        {
          ssi_buf[ssi_len] = 0;
          process_ssi (ssi_buf);
          ssi_len = 0;
          ssi_seq = 0;
          parser_state = 0;
        }
      }
      else
      {
        // SSI request continues
        for (int i = 0; i < ssi_seq; i++)
        {
          if (ssi_len < sizeof (ssi_buf) - 1)
            ssi_buf[ssi_len++] = ssi_end[i];
          else
          {
            // malformed SSI request
            ssi_buf[ssi_len] = 0;
            ws << ssi_intro;
            ws << ssi_buf;
            ssi_len = 0;
            ssi_seq = 0;
            parser_state = 0;
            if (!ws.good ())
              return -2;
            break;
          }
        }
        if (parser_state == 1)
        {
          ssi_buf[ssi_len++] = c;
          if (ssi_len == sizeof (ssi_buf) - 1)
          {
            // malformed SSI request
            ssi_buf[ssi_len] = 0;
            ws << ssi_intro;
            ws << ssi_buf;
            ssi_len = 0;
            ssi_seq = 0;
            parser_state = 0;
          }
        }
      }
      break;
    }
  }
  fclose (fn);
  return 0;
}

void http_connection::process_ssi (const char* req)
{
  while (*req && *req == ' ')
    req++;

  if (!strnicmp (req, "echo", 4))
  {
    req += 4;
    char buf[256];
    while (req)
    {
      req = strstr (req, "var=\"");
      if (req)
      {
        req += 5;
        int i = 0;
        while (*req && *req != '\"')
          buf[i++] = *req++;
        buf[i] = 0;
        if (*req)
          ws << parent.get_var (buf);
      }
    }
  }
}

/*!
  Send the content of a buffer.

  \param  src_buff buffer to send
  \param  sz buffer length

  \return 0 if successful or one of the following codes:
  \retval HTTPD_ERR_WRITE  - socket write failure

  The buffer is preceded by "Content-Length" header.
*/
int http_connection::serve_buffer (const BYTE* src_buff, size_t sz)
{
  int ret = HTTPD_OK;

  // find file size
  unsigned int len;
  len = (unsigned int)sz;
  TRACE8 ("http_connection::serve_buffer - size %d", len);
  char flen[30];
  sprintf (flen, "%d", len);
  add_ohdr ("Content-Length", flen);
  respond (200);
  const BYTE* buf = src_buff;
  unsigned int cnt = 0;
  while (cnt < len)
  {
    int out = cnt + 1024 < len ? 1024 : len - cnt;
    ws.write ((const char*)buf, out);
    if (!ws.good ())
    {
      TRACE2 ("socket write failure");
      ret = HTTPD_ERR_WRITE;
      break;
    }
    cnt += out;
    buf += out;
  }

  return ret;
}

/*!
  Send the content of a string.

  \param  str string to send

  \return 0 if successful or one of the following codes:
  \retval HTTPD_ERR_WRITE  - socket write failure

  The string is preceded by "Content-Length" header.
*/
int http_connection::serve_buffer (const std::string& str)
{
  return serve_buffer ((const BYTE*)str.data (), str.size ());
}

/*!
  Send the content of a file.

  \param  full_path fully qualified path name (UTF8 encoded)

  \return 0 if successful or one of the following codes:
  \retval HTTPD_ERR_WRITE  - socket write failure
  \retval HTTPD_ERR_FOPEN  - file open failure
  \retval HTTPD_ERR_FREAD  - file read failure

  The file is preceded by "Content-Length" header.
*/
int http_connection::serve_file (const std::string& full_path)
{
  int ret = HTTPD_OK;
#ifdef MLIB_HAS_UTF8_LIB
  FILE* fin = utf8::fopen (full_path, "rbS");
#else
  FILE* fin = fopen (full_path.c_str (), "rbS");
#endif
  if (!fin)
  {
    TRACE2 ("File %s - open error", full_path.c_str ());
    return HTTPD_ERR_FOPEN;
  }
  // find file size
  unsigned int len;
  fseek (fin, 0, SEEK_END);
  len = ftell (fin);
  fseek (fin, 0, SEEK_SET);
  TRACE8 ("http_connection::serve_file - File %s size %d", full_path.c_str (), len);
  char flen[30];
  sprintf (flen, "%d", len);
  add_ohdr ("Content-Length", flen);
  respond (200);
  if (strcmpi (get_method (), "HEAD"))
  {
    char buf[1024];
    int cnt = 1;
    while (cnt)
    {
      cnt = (int)fread (buf, 1, sizeof (buf), fin);
      if (!cnt && ferror (fin))
      {
        TRACE2 ("File %s - file read error %d", full_path.c_str (), ferror (fin));
        ret = HTTPD_ERR_FREAD;
      }
      ws.write (buf, cnt);
      if (!ws.good ())
      {
        TRACE2 ("File %s - socket write failure", full_path.c_str ());
        ret = HTTPD_ERR_WRITE;
        break;
      }
    }
  }
  fclose (fin);

  return ret;
}

bool http_connection::parse_url ()
{
  // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
  char* ptr = request;

  // method
  while (*ptr != ' ' && *ptr != '\n' && *ptr)
    ptr++;
  if (*ptr != ' ')
    return false;
  *ptr++ = 0;

  // SP
  while (*ptr == ' ')
    ptr++;

  // Request-URI
  uri = ptr;
  while (*ptr != ' ' && *ptr != '\n' && *ptr)
    ptr++;
  if (*ptr != ' ')
    return false;
  *ptr++ = 0;

  // SP
  while (*ptr == ' ')
    ptr++;

  // HTTP-Version
  http_version = ptr;
  while (*ptr != '\n' && *ptr)
    ptr++;
  if (*ptr != '\n')
    return false;
  *ptr++ = 0;

  // headers start on next line
  headers = ptr;

  // parse URI
  if (*uri == '/')
    uri++;
  ptr = uri;
  while (*ptr)
  {
    if (*ptr == '?' && !query)
    {
      *ptr++ = 0;
      query = ptr;
    }
    else if (*ptr == '#')
      *ptr = 0;
    else
      ptr++;
  }

  return true;
}

/*!
  Add or modify a response header.

  \param  hdr   header name
  \param  value header value

  To have any effect, this function should be called before calling the
  response() (or serve_...) function as all headers are sent at that time.
*/
void http_connection::add_ohdr (const char* hdr, const char* value)
{
  oheaders[hdr] = value;
}

/*!
  Generate a HTTP redirect response to a new uri.

  \param  uri   redirected uri
  \param  code  redirect code
*/
void http_connection::redirect (const char* uri, unsigned int code)
{
  if (!uri || !strlen (uri))
    uri = "/";

  add_ohdr ("Location", uri);
  respond (code);
  if (code == 303 && strcmpi ("HEAD", get_method ()))
  {
    /* RFC7231 (https://tools.ietf.org/html/rfc7231#section-6.4.4) says:
     Except for responses to a HEAD request, the representation of a 303
     response ought to contain a short hypertext note with a hyperlink to
     the same URI reference provided in the Location header field
     */
    ws << "<html><head><title>Document has moved</title></head>\r\n"
          "<body>Document has moved <a href=\""
       << uri << "\">here</a></body></html>" << endl;
  }
}

/*!
  Send the beginning of HTTP response.

  \param  code HTTP response code
  \param  reason reason-phrase. If NULL a standard reason-phrase is used.

  First time the function sends the status line and headers of the HTTP response:
  \verbatim
    HTTP/1.1 <code> <text>
    ...
    server headers
    ...
    connection headers
  \endverbatim

  In subsequent calls it sends only connection headers (to support multi-part responses)
*/
void http_connection::respond (unsigned int code, const char* reason)
{
  static struct
  {
    int code;
    const char* text;
  } respcodes[] = {{100, "Continue"},
                   {101, "Switching Protocols"},
                   {200, "OK"},
                   {201, "Created"},
                   {202, "Accepted"},
                   {203, "Non-Authoritative Information"},
                   {204, "No Content"},
                   {205, "Reset Content"},
                   {206, "Partial Content"},
                   {300, "Multiple Choices"},
                   {301, "Moved Permanently"},
                   {302, "Found"},
                   {303, "See Other"},
                   {304, "Not Modified"},
                   {305, "Use Proxy"},
                   {307, "Temporary Redirect"},
                   {400, "Bad Request"},
                   {401, "Unauthorized"},
                   {403, "Forbidden"},
                   {404, "Not Found"},
                   {405, "Method Not Allowed"},
                   {406, "Not Acceptable"},
                   {407, "Proxy Authentication Required"},
                   {408, "Request Time-out"},
                   {409, "Conflict"},
                   {410, "Gone"},
                   {411, "Length Required"},
                   {412, "Precondition Failed"},
                   {413, "Request Entity Too Large"},
                   {414, "Request-URI Too Large"},
                   {415, "Unsupported Media Type"},
                   {416, "Requested range not satisfiable"},
                   {417, "Expectation Failed"},
                   {500, "Internal Server Error"},
                   {501, "Not Implemented"},
                   {502, "Bad Gateway"},
                   {503, "Service Unavailable"},
                   {504, "Gateway Time-out"},
                   {505, "HTTP Version not supported"},
                   {0, 0}};

  str_pairs::iterator idx;
  if (code != 200)
    TRACE ("response %d\n", code);
  if (!response_sent)
  {
    int ic = 0;
    while (respcodes[ic].code && respcodes[ic].code != code)
      ic++;
    if (!respcodes[ic].code)
      return; // unknown code

    ws << "HTTP/1.1 " << code << " " << (reason ? reason : respcodes[ic].text) << "\r\n";

    // output server headers
    lock l (parent.hdr_lock);
    idx = parent.out_headers.begin ();
    while (idx != parent.out_headers.end ())
    {
      ws << idx->first << ": " << idx->second << "\r\n";
      idx++;
    }
    response_sent = true;
  }
  // followed by our headers
  TRACE9 ("Sending connection headers");
  idx = oheaders.begin ();
  while (idx != oheaders.end ())
  {
    ws << idx->first << ": " << idx->second << "\r\n";
    idx++;
  }
  ws << "\r\n";
  ws.flush ();
}

/*!
  Returns the value of a request header.

  \param  hdr   header name
  \return header value or 0 if the request does not have this header
*/
const char* http_connection::get_ihdr (const char* hdr) const
{
  char tmp[256];
  strcpy (tmp, hdr);
  auto idx = iheaders.find (_strlwr (tmp));
  if (idx != iheaders.end ())
    return idx->second.c_str ();

  return 0;
}

/*!
  Returns the value of a response header. The header can belong either to server
  or to connection.

  \param  hdr  header name

  \return header field value or 0 if there is no such header defined.
*/
const char* http_connection::get_ohdr (const char* hdr) const
{
  lock l (parent.hdr_lock);
  auto idx = parent.out_headers.find (hdr);
  if (idx != parent.out_headers.end ())
    return idx->second.c_str ();

  auto idx1 = oheaders.find (hdr);
  if (idx1 != oheaders.end ())
    return idx1->second.c_str ();

  return 0;
}

/*!
  Send first part of a multi-part response.
  \param part_type value of the 'Content-Type' header
  \param bound part boundary string
*/
void http_connection::respond_part (const char* part_type, const char* bound)
{
  part_boundary = bound;
  string mpart (part_type);
  mpart += ";boundary=";
  mpart += part_boundary;
  add_ohdr ("Content-Type", mpart.c_str ());
  respond (200);
  ws << "--" << part_boundary << "\r\n";
  ws.flush ();
}

/*!
  Send subsequent parts of a multi-part response.
  \param last _true_ if this is the last part of the mulit-part response

  This function should be called only after a call to respond_part() function.
*/
void http_connection::respond_next (bool last)
{
  if (part_boundary.empty ())
    return; // misuse
  ws << "\r\n--" << part_boundary;
  if (last)
  {
    ws << "--";
    part_boundary.clear ();
  }
  ws << "\r\n";
}

//-----------------------------------------------------------------------------
/*!
  \class httpd
  \ingroup sockets

  This class is derived from tcpserver class and implements a basic HTTP server.
  After construction, the main server thread has to be started by calling start()
  function. When started, the server binds to the listening socket and creates
  new http_connection objects for each incoming client. All the protocol is then
  handled by the http_connection (or derived) class.

  A HTTP server can be integrated to an application by adding specific url handlers
  and user variables. URL handlers are registered by calling the add_handler() function.

  User variables can be added by calling the add_var() function. The content of
  those variables is then returned in response to SSI echo directives.
*/

/*!
  Constructor.
  \param port     listening port
  \param maxconn  maximum number of incoming connections. If 0 the number of
                  connections is unlimited

  If \p port is 0, when the server is started it will bind to an available port
  __on local interface only__.

  - document root is current folder (".")
  - default url is HTTPD_DEFAULT
  - server name is HTTPD_SERVER_NAME
*/
httpd::httpd (unsigned short port, unsigned int maxconn)
  : tcpserver (port, HTTPD_SERVER_NAME, maxconn)
  , root ("./")
  , defuri (HTTPD_DEFAULT_URI)
{
  smime* ptr = knowntypes;
  while (ptr->suffix)
  {
    add_mime_type (ptr->suffix, ptr->type, ptr->shtml);
    ptr++;
  }
}

/*!
  Destructor
*/
httpd::~httpd ()
{}

/*!
  Change server name.
  \param nam new server name string

  This string is returned in the "Server" HTTP header.
*/
void httpd::name (const char* nam)
{
  if (nam)
    add_ohdr ("Server", nam);
  else
    remove_ohdr ("Server");
  thread::name (nam);
}

/*!
  Create an new http_connection object for a new incoming connection

  Derived classes can override this function to return objects derived from
  http_connection class.
*/
http_connection* httpd::make_thread (sock& connection)
{
  return new http_connection (connection, *this);
}

/*!
  Add or modify a server response header.
  \param hdr      Header name
  \param value    Header value

  Server response headers are always sent as part of the HTTP answer.
  In addition each connection object can add it's own headers.
*/
void httpd::add_ohdr (const char* hdr, const char* value)
{
  lock l (hdr_lock);
  out_headers[hdr] = value;
}

/*!
  Remove a server response header.
  \param hdr    Header name
*/
void httpd::remove_ohdr (const char* hdr)
{
  lock l (hdr_lock);
  auto idx = out_headers.find (hdr);
  if (idx != out_headers.end ())
    out_headers.erase (idx);
}

/*!
  Add or modify an URI handler function.
  \param uri    URI address
  \param func   handler function
  \param info   handler specific information

  When a HTTP client requests the URI, the connection object will invoke the
  user-defined handler function passing the uri, the connection info and the
  handler specific information.
*/
void httpd::add_handler (const char* uri, uri_handler func, void* info)
{
  handle_info hi = {func, info};
  handlers.emplace (string (uri), hi);
}

/*!
  Add/change content of table matching MIME types to file extensions
  \param  ext   filename extension
  \param  type  MIME type
  \param  shtml true if SSI processing should be enabled for this file type
*/
void httpd::add_mime_type (const char* ext, const char* type, bool shtml)
{
  mimetype t;
  deque<mimetype>::iterator ptr = types.begin ();
  while (ptr != types.end ())
  {
    if (!strcmpi (ptr->suffix.c_str (), ext))
    {
      ptr->type = type;
      ptr->shtml = shtml;
      return;
    }
    ptr++;
  }

  t.suffix = ext;
  t.type = type;
  t.shtml = shtml;

  types.push_back (t);
}

/*!
  Remove a file type from the MIME type table
  \param ext    filename extension
*/
void httpd::delete_mime_type (const char* ext)
{
  deque<mimetype>::iterator ptr = types.begin ();
  ptr++; // the first (default value) cannot be deleted
  while (ptr != types.end ())
  {
    if (!strcmpi (ptr->suffix.c_str (), ext))
    {
      types.erase (ptr);
      return;
    }
    ptr++;
  }
}

/*!
  Add a new access realm. Realms are assigned to specific uri paths and their
  access can be restricted to specified users.
  \param realm    protection realm
  \param uri      starting path
*/
void httpd::add_realm (const char* realm, const char* uri)
{
  if (*uri == '/')
    uri++;
  realms[realm] = uri;
}

/*!
  Add a new user to a relm or modifies password for an existing user.
  \param realm access realm
  \param username user name
  \param pwd user password
  \return _true_ if successful or _false_ if realm doesn't exist
*/
bool httpd::add_user (const char* realm, const char* username, const char* pwd)
{
  if (realms.find (realm) == realms.end ())
    return false; // no such realm
  user inf;
  inf.name = username;
  inf.pwd = pwd;
  multimap<string, user>::iterator it = credentials.find (realm);
  if (it == credentials.end ())
  {
    pair<string, user> p (realm, inf);
    credentials.insert (p);
  }
  else
  {
    while (it != credentials.end () && it->first == realm)
    {
      if (it->second.name == username)
      {
        it->second.pwd = pwd; // change password
        return true;
      }
      it++;
    }
    pair<string, user> p (realm, inf);
    credentials.insert (p);
  }

  return true;
}

bool httpd::remove_user (const char* realm, const char* username)
{
  if (realms.find (realm) == realms.end ())
    return false; // no such realm

  multimap<string, user>::iterator it = credentials.find (realm);

  while (it != credentials.end () && it->first == realm)
  {
    if (it->second.name == username)
    {
      credentials.erase (it); // remove user
      return true;
    }
    it++;
  }

  return true;
}

/*!
  Find the longest match realm that covers an uri
  \param uri    uri to check
  \param realm  matching realm
  \return       true if uri is covered by a realm

*/

bool httpd::is_protected (const char* uri, string& realm)
{
  if (*uri == '/')
    uri++;
  int len = -1;
  auto it = realms.begin ();
  while (it != realms.end ())
  {
    int n = match (uri, it->second.c_str ());
    if (n == it->second.length () && n > len)
    {
      len = n;
      realm = it->first;
    }
    it++;
  }
  return (len >= 0);
}

/*!
  Verify user credentials for a realm.
  \return     true if user is authorized for the realm
*/
bool httpd::authenticate (const char* realm, const char* user, const char* pwd)
{
  auto it = credentials.find (realm);
  if (it == credentials.end ())
    return false; // no such realm
  while (it != credentials.end () && it->first == realm)
  {
    if (it->second.name == user && it->second.pwd == pwd)
      return true;
    it++;
  }
  return false;
}

/*!
  Invoke a user defined URI handler
  \param  uri    URI that triggered the handler invocation
  \param  client connection thread

  \return the result of calling the user handler or 0 if there is no handler
          set for the URI.

*/
int httpd::invoke_handler (const char* uri, http_connection& client)
{
  int ret = 0;
  auto idx = handlers.find (uri);
  if (idx != handlers.end ())
  {
    lock l (*idx->second.in_use);
    TRACE9 ("Invoking handler for %s", uri);
    ret = idx->second.h (uri, client, idx->second.nfo);
    TRACE9 ("Handler done (%d)", ret);
  }

  return ret;
}

/*!
  Maps a local file path to a path in the URI space.
  \param  uri     name in URI space
  \param  path    mapped local file path
*/
void httpd::add_alias (const char* uri, const char* path)
{
  aliases[uri] = path;
}

/*!
  Retrieve the local file path mapped to an URI
  \param  uri    URI path
  \param  path   local path

  \return true if URI was successfully mapped.

  After processing the alias table, any part of the original URI that was not
  mapped is appended to the resulting path. For instance if the alias table
  contains an entry mapping "doc" to "documentation" and docroot is set as
  "c:\local_folder\", an URI like "/doc/project1/filename.html" will be mapped to
  "c:\local_folder\documentation\project1\filename.html".
*/
bool httpd::find_alias (const char* uri, char* path)
{
  auto idx = aliases.begin ();
  while (idx != aliases.end ())
  {
    size_t len = idx->first.length ();
    if (!strncmp (idx->first.c_str (), uri, len) && uri[len] == '/')
    {
      strcpy (path, idx->second.c_str ());
      strcat (path, uri + len);
      return true;
    }
    idx++;
  }
  return false;
}

/*!
  Add or modify a user variable
  \param name     variable name (the name used in SSI construct)
  \param fmt      sprintf format string
  \param addr     address of content
  \param multiplier for numeric variables, resulting value is multiplied by this factor

  User variables are accessible through SSi constructs like:
  \verbatim
    <!--#echo var="name" -->
  \endverbatim

  When the page is served the SSI construct is replaced by the current value of
  the named variable, eventually multiplied by \a multiplier factor and formatted
  as text using the \a fmt string.
*/
void httpd::add_var (const char* name, const char* fmt, void* addr, double multiplier)
{
  struct var_info vi = {fmt, addr, multiplier};
  lock l (varlock);
  variables[name] = vi;
}

/*!
  Return the current string representation of a variable.
  \param name variable name
*/
string httpd::get_var (const char* name)
{
  lock l (varlock);
  if (variables.find (name) != variables.end ())
  {
    struct var_info vi = variables[name];
    char buf[256];
    int vt = fmt2type (vi.fmt.c_str ());

    switch (vt)
    {
    case 0:
      sprintf (buf, vi.fmt.c_str (), vi.addr);
      break;
    case 1:
      sprintf (buf, vi.fmt.c_str (), *(int*)vi.addr);
      break;
    case 2:
      sprintf (buf, vi.fmt.c_str (), *(long*)vi.addr);
      break;
    case 3:
      sprintf (buf, vi.fmt.c_str (),
               (vi.multiplier == 0.) ? *(float*)vi.addr
                                     : (*(float*)vi.addr * (float)vi.multiplier));
      break;
    case 4:
      sprintf (buf, vi.fmt.c_str (),
               (vi.multiplier == 0.) ? *(double*)vi.addr : (*(double*)vi.addr * vi.multiplier));
      break;
    default:
      buf[0] = 0;
    }
    return string (buf);
  }
  else
    return string ("none");
}

/*!
  Set server root path
*/
void httpd::docroot (const char* path)
{
  root = path;
  // make sure it's (back)slash terminated
  if (root.back () != '\\' && root.back () != '/')
    root.push_back ('/');
}
/*!
  Guess MIME type of a file and if SSI replacement should be enabled based on
  file extension.
  \param file file name
  \param shtml return true if SSI replacement should be enabled for this file type

  \return MIME type
*/
const char* httpd::guess_mimetype (const char* file, bool& shtml)
{
  const char* ext = strrchr (file, '.');
  auto ptr = types.begin ();

  if (ext)
  {
    ext++;
    while (ptr != types.end () && strcmpi (ptr->suffix.c_str (), ext))
      ptr++;
    if (ptr != types.end ())
    {
      shtml = ptr->shtml;
      return ptr->type.c_str ();
    }
  }
  shtml = false;
  return types[0].type.c_str ();
}

/// Check if file exists and can be read
bool file_exists (char* path)
{
  // normalize forward slashes to backslashes
  char* p = path;
  while (*p)
  {
    if (*p == '/')
      *p = '\\';
    ++p;
  }
#ifdef MLIB_HAS_UTF8_LIB
  return utf8::access (path, 4);
#else
  return (_access (path, 4) != 0);
#endif
}

/*!
  Return type requested by a format string
  \return One of the following codes:
  \retval  0 - string
  \retval  1 - int
  \retval  2 - long int
  \retval  3 - float
  \retval  4 - double
  \retval  5 - character
  \retval -1 - unknown
*/
int fmt2type (const char* fmt)
{
  int ret = -1;
  fmt = strchr (fmt, '%');
  if (fmt)
  {
    fmt += strcspn (fmt, "sfegidxuc"); // points to first format char or end of string
    switch (*fmt)
    {
    case 's':
      ret = 0;
      break; // string
    case 'c':
      ret = 5;
      break;
    case 'f':
    case 'e':
    case 'g':
      ret = (tolower (*(fmt - 1)) == 'l') ? 4 : 3;
      break; // float

    case 0:
      ret = -1;
      break; // unknown

    default:
      ret = (tolower (*(fmt - 1)) == 'l') ? 2 : 1;
      break; // integer
    }
  }
  return ret;
}

int hexdigit (char* bin, char c)
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

int hexbyte (char* bin, const char* str)
{
  char d1, d2;

  // first digit
  if (!hexdigit (&d1, *str++) || !hexdigit (&d2, *str++))
    return 0;
  *bin = (d1 << 4) | d2;
  return 1;
}

/*!
  In place decoding of URL-encoded data.
  We can do it in place because resulting string is shorter or equal than input.

  \return 1 if successful, 0 otherwise
*/
int url_decode (char* buf)
{
  char *in, *out;

  in = out = buf;

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

/*!
  Return number of matching characters at beginning of str1 and str2.
  Comparison is case insensitive.
*/
int match (const char* str1, const char* str2)
{
  int n = 0;
  while (*str1 && *str2 && tolower (*str1++) == tolower (*str2++))
    n++;
  return n;
}

/// Parse a URL encoded string of parameters
void parse_urlparams (const char* par_str, str_pairs& params)
{
  char *ptr, *val, *next;
  char k[MAX_PAR], v[MAX_PAR];

  char* dup = strdup (par_str);
  for (ptr = dup; ptr && *ptr; ptr = next)
  {
    next = strchr (ptr, '&');
    if (next)
      *next++ = 0;

    if (val = strchr (ptr, '='))
    {
      *val++ = 0;
      strcpy (v, val);
      url_decode (v);
    }
    else
      *v = 0;
    strcpy (k, ptr);
    url_decode (k);
    params[k] = v;
  }
  free (dup);
}

} // namespace mlib

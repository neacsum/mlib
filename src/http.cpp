/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file http.cpp Implementation of http::server and http::connection classes
#include <mlib/mlib.h>
#pragma hdrstop

#include <utf8/utf8.h>
#include <utils.h>

using namespace std;
using namespace mlib;

// Defaults
#define HTTP_DEFAULT_URI "index.html"      //!< Default URL
#define HTTP_SERVER_NAME "MLIB_HTTP 2.0"   //!< Default server name

/// Max length of a form parameter
#define MAX_PAR 1024

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

namespace mlib::http {

/// Table of known mime types
struct smime
{
  std::string type;
  bool shtml;
};

std::map<std::string, smime> static knowntypes {
  {"htm",   {"text/html", false}}, //also default mime type
  {"html",  {"text/html", false}},
  {"shtml", {"text/html", true}},
  {"shtm",  {"text/html", true}},
  {"css",   {"text/css", false}},
  {"xml",   {"text/xml", false}},
  {"json",  {"text/json", false}},
  {"gif",   {"image/gif", false}},
  {"jpg",   {"image/jpeg", false}},
  {"jpeg",  {"image/jpeg", false}},
  {"png",   {"image/png", false}},
  {"ico",   {"image/png", false}},
  {"svg",   {"image/svg+xml", false}},
  {"bmp",   {"image/bmp", false}},
  {"pdf",   {"application/pdf", false}},
  {"xslt",  {"application/xml", false}},
  {"js",    {"application/x-javascript", false}}
};

static size_t match (const std::string& str1, const std::string& str2);

//-----------------------------------------------------------------------------
/*!
  \class mlib::http::connection
  \ingroup  sockets

  This is the thread created by the mlib::http::server object in response to a
  new connection request.

  The thread is started automatically by the server. It listens on the 
  connection socket and process HTTP requests. When the HTTP client closes the
  connection the thread stops and the object is destructed by the server.

  Users can create derived classes but the parent server must also be a class
  derived from http::server with an overridden http::server::make_thread function.

  For details of request processing, see the run() function.

  Most of the public functions are designed for the benefit of user handler
  functions. The handler function, if one has been registered, is called before
  sending any reply to the HTTP client.
*/

/*!
  Protected constructor used by mlib::http::server class to create a new
  connection thread

  \param  socket  connecting socket
  \param  server  parent server object

  As it has only a protected constructor, it is not possible for users of this
  class to directly create this object. Derived classes should maintain this
  convention.

*/
connection::connection (sock& socket, server& server)
  : thread ("http::connection")
  , parent (server)
  , ws (socket)
  , response_sent (false)
  , content_len (-1)
  , query_parsed (false)
  , body_parsed (false)
{
}

/*!
  The loop has the following steps:

  1. Wait for a client request line and validate it.
  2. Get request headers; validate and parse them.
  3. For POST and PUT requests, get the request body.
  4. Invoke the handler function if one has been defined.
  5. Otherwise, see if it is a request for an existing file and return it.


*/
void connection::run ()
{
  ws->recvtimeout (HTTP_TIMEOUT);
  TRACE8 ("http_connection::run - Connection from %s", ws->peer ()->hostname ().c_str ());
  try
  {
    while (1)
    {
      std::string input;
      request_init ();

      // Accumulate HTTP request line
      while (input.size () < HTTP_MAX_HEADER)
      {
        // wait for chars.
        char ch = ws.get ();

        if (ws.eof ())
        {
          TRACE2 ("http::connection::run - client closed");
          return; // client closed
        }

        if (!ws.good ())
        {
          TRACE2 ("http_connection::run - timeout peer %s", ws->peer()->hostname ().c_str ());
          respond (408);
          return;
        }

        if (ch == '\r')
          continue;

        input.push_back (ch);
        if (ch == '\n')
          break;
      }
      if (input.size () == HTTP_MAX_HEADER)
      {
        TRACE2 ("http::connection::run - Request too long (%d) peer %s", input.size (),
                ws->peer()->hostname ().c_str ());
        respond (414);
        return;
      }
      if (!parse_request (input))
      {
        respond (400);
        return; // invalid request line
      }

      //Accumulate headers
      bool eol_seen = false;
      input.clear ();
      while (input.size () < HTTP_MAX_HEADER)
      {
        // wait for chars.
        char ch = ws.get ();

        if (ws.eof ())
        {
          TRACE2 ("http::connection::run - client closed");
          return; // client closed
        }

        if (!ws.good ())
        {
          TRACE2 ("http::connection::run - timeout peer %s", 
            ws->peer ()->hostname ().c_str ());
          respond (408);
          return;
        }

        if (ch == '\r')
          continue;

        if (ch == '\n')
        {
          if (eol_seen)
            break; // end of headers
          eol_seen = true;
        }
        else
          eol_seen = false;
        input.push_back (ch);
      }

      if (input.size () == HTTP_MAX_HEADER)
      {
        TRACE2 ("http_connection::run - Request too long peer %s",
                ws->peer ()->hostname ().c_str ());
        respond (413);
        return;
      }
      if (!parse_headers (input))
      {
        respond (400);
        return;
      }

      int auth_stat = do_auth ();
      if (auth_stat <= 0)
        return; // bad auth - response sent by do_auth

      //Prepare output headers
      if (!has_ihdr ("Connection"))
      {
        // Default "Connection" setting
        if (http_version.empty () || http_version == "HTTP/1.0")
          add_ohdr ("Connection", "close");
        else
          add_ohdr ("Connection", "keep-alive");
      }
      else
        add_ohdr ("Connection", get_ihdr ("Connection"));

      if (get_ohdr ("Connection") == "keep-alive")
      {
        /* set timeout to whatever client wants but not more than what the
           server accepts */
        unsigned int ka_srv = parent.keep_alive ();
        unsigned int ka_cli = UINT_MAX;
        if (has_ihdr ("Keep-Alive"))
        {
          auto ka_str = get_ihdr ("Keep-Alive");
          auto t = ka_str.find ("timeout=");

          if (t != string::npos)
            ka_cli = stoi (ka_str.substr (t));
        }
        ka_cli = min (ka_srv, ka_cli);
        ws->recvtimeout (ka_cli);
        add_ohdr ("Keep-Alive", "timeout="s + to_string (ka_cli));
      }
      if (method_ == "POST" || method_ == "PUT")
      {
        // read request body
        string cl = get_ihdr ("Content-Length");
        try
        {
          size_t iend;
          content_len = stoi (cl, &iend);
          if (iend != cl.length () || content_len < 0)
            throw std::out_of_range ("invalid number");
          else if (content_len > 0)
          {
            body.resize (content_len);
            ws.read (body.data (), content_len);
          }
        }
        catch (std::exception& x)
        {
          TRACE2 ("http::connection::run - Invalid content length (%s) peer %s cause %s",
                  cl.c_str (),
                  ws->peer()->hostname ().c_str (), x.what());
          body.clear ();
          respond (400); // invalid "Content-Length" header
          return;
        }
      }
      process_valid_request ();

      // should close connection?
      if (should_close ())
        return;
      ws.flush ();
    }
  }
  catch (erc& err)
  {
    respond (500);
    TRACE ("http::connection errcode=%d\n", err.code ());
  }
  catch (std::exception& err)
  {
    respond (500);
    TRACE ("http::connection runtime error %s\n", err.what ());
  }

  // all cleanup is done by term function
}

// Return true if client connection should be closed
bool connection::should_close ()
{
  if (has_ihdr ("Connection") && get_ihdr ("Connection") == "close")
  {
    TRACE8 ("should_close - client wants to close");
    return true; // client wants to close
  }
  if (has_ohdr ("Connection") && get_ohdr ("Connection") == "close")
  {
    TRACE8 ("should_close - server wants to close");
    return true; // server wants to close
  }
  if (method_ == "GET" && !has_ohdr ("Content-Length"))
  {
    TRACE8 ("should_close - no content length");
    return true; // don't have content length in response to GET (shtml file)
  }

  if (http_version == "HTTP/1.0"
   && (!has_ihdr ("Connection") || get_ihdr ("Connection") != "keep-alive"))
  {
    TRACE8 ("should_close - protocol HTTP 1.0");
    return true;
  }

  return false;
}

// initialization before getting a new client request
void connection::request_init ()
{
  content_len = -1;
  iheaders.clear ();
  qparams.clear ();
  bparams.clear ();
  body.clear ();

  // Response headers are initialized with server's response headers.
  lock l (parent.hdr_lock);
  oheaders = parent.out_headers;

  query_parsed = body_parsed = response_sent = false;
}

void connection::term ()
{
  TRACE8 ("http_connection::term - Closed connection to %s", ws->peer ()->hostname ().c_str ());
  ws.flush ();
  parent.close_connection (*ws.rdbuf ());
  thread::term ();
}

/*
  Check if path_ is covered by a server realm and if user has credentials for
  said realm.

  \return
    0   = Missing authorization (401)
   -1   = Bad authorization (501)
    1   = all good

*/
int connection::do_auth ()
{
  string realm;
  if (!parent.is_protected (path_, realm))
    return 1;

  if (!has_ihdr("Authorization"))
  {
    serve401 (realm.c_str ());
    return 0;
  }

  auto& auth = get_ihdr ("Authorization");
  if (auth.substr(0,6) != "Basic ")
  {
    // Other auth methods
    TRACE8 ("Authorization: %s", auth.c_str());
    respond (501); // not implemented
    return -1;
  }

  char buf[256];
  size_t outsz = base64dec (auth.substr (6).c_str (), buf);
  buf[outsz] = 0;
  char* pwd = strchr (buf, ':');
  if (pwd)
    *pwd++ = 0;
  else
    pwd = buf;
  if (!parent.authenticate (realm.c_str (), buf, pwd))
  {
    // invalid credential
    TRACE2 ("http::connection: Invalid credentials user %s", buf);
    serve401 (realm.c_str ());
    return 0;
  }
  TRACE8 ("http::connection: Authenticated user %s for realm %s", buf, realm.c_str ());
  return 1;
}

void connection::serve401 (const char* realm)
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
void connection::process_valid_request ()
{
  if (method_ == "OPTIONS")
  {
    serve_options ();
    return;
  }
  
  // check if there is a handler registered for this URI
  if (parent.invoke_handler (*this) == HTTP_OK)
  {
    TRACE9 ("handler done");
    return;
  }

  if (method_ == "GET" || method_ == "HEAD")
  {
    // try to see if we can serve a file
    std::filesystem::path fullpath;
    if (parent.locate_resource (path_, fullpath))
    {
      bool shtml = false;
      int ret;
      if (!has_ohdr ("Content-Type"))
        add_ohdr ("Content-Type", parent.guess_mimetype (fullpath, shtml));
      if (shtml)
        ret = serve_shtml (fullpath);
      else
        ret = serve_file (fullpath);
      if (ret == -2)
        respond (403);
      TRACE9 ("served %s result = %d", fullpath.string ().c_str (), ret);
    }
    else
    {
      TRACE2 ("not found %s", fullpath.string ().c_str());
      serve404 ();
    }
  }
  else if (method_ == "POST" || method_ == "PUT")
  {
    if (!parent.invoke_post_handler (*this))
      respond (204);
  }
  else
    respond (400);
}

// Response to OPTIONS request
void connection::serve_options ()
{
  std::filesystem::path fullpath;
  if (path_ == "*")
  {
    add_ohdr ("Allow", "OPTIONS, GET, HEAD, POST, PUT");
    respond (204);
  }
  else if (parent.post_handlers.find (path_) != parent.post_handlers.end ())
  {
    add_ohdr ("Allow", "OPTIONS, PUT, POST");
    respond (204);
  }
  else if (parent.locate_resource (path_, fullpath))
  {
    add_ohdr ("Allow", "OPTIONS, GET, HEAD");
    respond (204);
  }
  else if (parent.locate_handler (path_, nullptr))
  {
    add_ohdr ("Allow", "OPTIONS, GET, HEAD, POST, PUT");
    respond (204);
  }
  else
    serve404 ();
}

/*!
  Sends a 404 (page not found) response
*/
void connection::serve404 (const char* text)
{
  static const char* std404 = "<html><head><title>Page not found</title></head>"
                              "<body><h1>Oops! 404 - File not found</h1>"
                              "<p>The page you requested was not found.</body></html>";

  if (!text)
    text = std404;

  add_ohdr ("Content-Length", to_string (strlen(text)));
  add_ohdr ("Content-Type", "text/html");
  respond (404);
  ws << text;
}


bool connection::parse_headers (const std::string& hdrs)
{
  auto ptr = hdrs.begin();
  string key, val;
  int fsm_state = 0;
  iheaders.clear ();
  do
  {
    switch (fsm_state)
    {
    case 0: //initial header name character
      if (ptr == hdrs.end ())
        break; // 
      if (strchr (": \t\n", *ptr) != 0)
        fsm_state = -1;
      else
      {
        fsm_state = 1;
        key.push_back (*ptr++);
      }
      break;

    case 1: //continuation header name character
      if (strchr (" \t\n", *ptr) != 0)
        fsm_state = -1;
      else if (*ptr == ':')
      {
        fsm_state = 2;
        ++ptr;
      }
      else
        key.push_back (*ptr++);
      break;

    case 2: //initial header value character
      if (strchr (":\n", *ptr) != 0)
        fsm_state = -1;
      else if (*ptr == ' ' || *ptr == '\t')
        ++ptr;
      else
      {
        fsm_state = 3;
        val.push_back (*ptr++);
      }
      break;

    case 3: //continuation header value character
      if (*ptr == '\n')
      {
        while (val.back () == ' ' || val.back () == '\t')
          val.pop_back ();

        if (iheaders.find (key) != iheaders.end ())
        {
          if (key == "Host")
            return false; //multiple "Host" fields
          val = iheaders[key] + ',' + val;
        }
        iheaders[key] = val;
        key.clear ();
        val.clear ();
        fsm_state = 0;
      }
      else
        val.push_back (*ptr);
      ++ptr;
      break;
    }
  } while (ptr != hdrs.end () && fsm_state >= 0);
  
  if (fsm_state != 0)
    return false;

  //check for required headers
  if (http_version == "HTTP/1.1" && !has_ihdr ("Host"))
    return false;
  if (!has_ihdr ("Content-Length") && (method_ == "POST" || method_ == "PUT"))
    return false; // must have "Content-Length" header

  return true;
}

bool connection::parse_body ()
{
  // check if we have what to parse
  if (body.empty () || body_parsed  || !has_ihdr ("Content-Type"))
    return body_parsed;

  if (get_ihdr ("Content-Type") == "application/x-www-form-urlencoded")
  {
    parse_urlparams (body, bparams);
    body_parsed = true;
    return true;
  }
  // TODO - add parsing of other content types
  return false;
}

/// Parse an URL encoded query
void connection::parse_query ()
{
  if (query_parsed)
    return; // don't repeat parsing
  parse_urlparams (query_, qparams);
  query_parsed = true;
}


/*!
  Send the content of a file, processing any SSI directives.

  \param  file file name

  \return 0 if successful or one of the following values:
  \retval -1 socket write failure
  \retval -2 file open error

  As we don't know the size of the response, the file is sent without a
  "Content-Length" header.
*/
int connection::serve_shtml (const std::filesystem::path& file)
{
  auto fname = file.u8string ();
  FILE* fin = utf8::fopen (fname, "rbS");
  if (!fin)
  {
    TRACE2 ("File %s - open error", fname.c_str());
    return HTTP_ERR_FOPEN;
  }
  respond (200);

  if (get_method () == "HEAD")
  {
    fclose (fin);
    return 0; // don't send body in response to a HEAD request
  }
  int parser_state = 0;
  char ssi_buf[256];
  int ssi_len;
  int ssi_seq = 0;
  static char ssi_intro[] = "<!--#";
  static char ssi_end[] = "-->";
  while (!feof (fin))
  {
    char c = fgetc (fin);
    if (feof (fin))
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
  fclose (fin);
  return 0;
}

void connection::process_ssi (const char* req)
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
  \retval HTTP_ERR_WRITE  - socket write failure

  The buffer is preceded by "Content-Length" header.
*/
int connection::serve_buffer (const BYTE* src_buff, size_t sz)
{
  int ret = HTTP_OK;

  // find file size
  unsigned int len;
  len = (unsigned int)sz;
  TRACE8 ("http::connection::serve_buffer - size %d", len);
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
      ret = HTTP_ERR_WRITE;
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
  \retval HTTP_ERR_WRITE  - socket write failure

  The string is preceded by "Content-Length" header.
*/
int connection::serve_buffer (const std::string& str)
{
  return serve_buffer ((const BYTE*)str.data (), str.size ());
}

/*!
  Send the content of a file.

  \param  file file name

  \return 0 if successful or one of the following codes:
  \retval HTTP_ERR_WRITE  - socket write failure
  \retval HTTP_ERR_FOPEN  - file open failure
  \retval HTTP_ERR_FREAD  - file read failure

  The file is preceded by "Content-Length" header.
*/
int connection::serve_file (const std::filesystem::path& file)
{
  int ret = HTTP_OK;
  auto fname = file.u8string ();
  char buf[1024];
  FILE* fin = utf8::fopen (fname, "rbS");
  if (!fin)
  {
    TRACE2 ("File %s - open error", fname.c_str());
    return HTTP_ERR_FOPEN;
  }
  // find file size
  unsigned int len;
  fseek (fin, 0, SEEK_END);
  len = ftell (fin);
  fseek (fin, 0, SEEK_SET);
  TRACE8 ("http::connection::serve_file - File %s size %d", fname.c_str (), len);
  add_ohdr ("Content-Length", to_string(len));
  auto file_time = std::filesystem::last_write_time (file);
  auto tp = std::chrono::clock_cast<std::chrono::system_clock> (file_time);
  std::time_t cftime = std::chrono::system_clock::to_time_t (tp);
  auto tm = std::gmtime (&cftime);
  std::strftime (buf, sizeof (buf), "%a, %d %b %Y %H:%M:%S GMT", tm);
  add_ohdr ("Last-Modified", buf);
  respond (200);
  if (get_method() != "HEAD")
  {
    int cnt = 1;
    while (cnt)
    {
      cnt = (int)fread (buf, 1, sizeof (buf), fin);
      if (!cnt && ferror (fin))
      {
        TRACE2 ("File %s - file read error %d", fname.c_str (), ferror (fin));
        ret = HTTP_ERR_FREAD;
      }
      ws.write (buf, cnt);
      if (!ws.good ())
      {
        TRACE2 ("File %s - socket write failure", fname.c_str());
        ret = HTTP_ERR_WRITE;
        break;
      }
    }
  }
  fclose (fin);

  return ret;
}

bool connection::parse_request (const std::string& req)
{
  // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
  auto crt = req.begin();

  // method
  while (crt != req.end() && *crt != ' ' && *crt != '\n')
    crt++;
  if (*crt != ' ')
    return false;
  method_ = string (req.begin (), crt);
  str_upper (method_);

  // SP
  while (*++crt == ' ')
    ;

  // Request-target
  auto pbeg = crt;
  while (crt != req.end() && *crt != ' ' && *crt != '\n')
    ++crt;
  if (*crt != ' ')
    return false;
  
  // Request target must be in origin-form: absolute-path ["?" query] ["#" fragment]
  // or the request must be "OPTIONS *"
  if (*pbeg != '/' 
   && !(method_ == "OPTIONS" && *pbeg == '*' && (crt - pbeg == 1)))
    return false; 

  path_ = string (pbeg, crt);

  // SP
  while (*++crt == ' ')
    ;

  // HTTP-Version
  pbeg = crt;
  while (crt != req.end () && *crt != '\n')
    ++crt;
  if (*crt != '\n')
    return false;
  http_version = string (pbeg, crt);

  // separate query part
  auto qbeg= path_.find ('?');
  if (qbeg != string::npos)
  {
    query_ = path_.substr (qbeg + 1);
    auto qend = query_.find ('#');
    if (qend != string::npos)
      query_.erase (qend);
    path_.erase (qbeg);
    url_decode (query_); // normalization of query component
  }
  else
  {
    //query is missing but maybe there is a fragment
    auto pend = path_.find ('#');
    if (pend != string::npos)
      path_.erase (pend);
  }
  url_decode (path_); // path normalization (RFC9110 sect 4.2.3)

  return true;
}


/*!
  Generate a HTTP redirect response to a new resource.

  \param  uri   path for new resource
  \param  code  redirect code
*/
void connection::redirect (const std::string& uri, unsigned int code)
{
  add_ohdr ("Location", uri);
  respond (code);
  if (code == 303 && get_method () != "HEAD")
  {
    /* RFC9110 (https://www.rfc-editor.org/rfc/rfc9110#name-303-see-other)says:
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
  \param  reason reason-phrase. If empty a standard reason-phrase is used.

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
void connection::respond (unsigned int code, const std::string& reason)
{
  static struct
  {
    int code;
    const char* text;
  } respcodes[] = {
    {100, "Continue"},          {101, "Switching Protocols"},
    {200, "OK"},                {201, "Created"},
    {202, "Accepted"},          {203, "Non-Authoritative Information"},
    {204, "No Content"},        {205, "Reset Content"},
    {206, "Partial Content"},   {300, "Multiple Choices"},
    {301, "Moved Permanently"}, {302, "Found"},
    {303, "See Other"},         {304, "Not Modified"},
    {305, "Use Proxy"},         {307, "Temporary Redirect"},
    {400, "Bad Request"},       {401, "Unauthorized"},
    {403, "Forbidden"},         {404, "Not Found"},
    {405, "Method Not Allowed"},{406, "Not Acceptable"},
    {407, "Proxy Authentication Required"}, {408, "Request Time-out"},
    {409, "Conflict"},          {410, "Gone"},
    {411, "Length Required"},   {412, "Precondition Failed"},
    {413, "Request Entity Too Large"}, {414, "Request-URI Too Large"},
    {415, "Unsupported Media Type"}, {416, "Requested range not satisfiable"},
    {417, "Expectation Failed"},{500, "Internal Server Error"},
    {501, "Not Implemented"},   {502, "Bad Gateway"},
    {503, "Service Unavailable"},{504, "Gateway Time-out"},
    {505, "HTTP Version not supported"}, {0, 0}};

  if (response_sent)
    return; // already sent response

  if (code != 200)
    TRACE ("response %d - %s\n", code, reason.c_str());

  str_pairs::iterator idx;
  int ic = 0;

  while (respcodes[ic].code && respcodes[ic].code != code)
    ic++;
  ws << "HTTP/1.1 " << code;
  if (!reason.empty())
    ws << " " << reason;
  else if (respcodes[ic].code)
    ws << " " << respcodes[ic].text;
  ws << "\r\n";

  //If content type is not set, default to plain text 
  if (code != 204 && !has_ohdr ("Content-Type"))
    add_ohdr ("Content-Type", "text/plain");

  if (!has_ohdr ("Date"))
  {
    char buf[80];
    time_t cftime = chrono::system_clock::to_time_t (chrono::system_clock::now());
    auto tm = gmtime (&cftime);
    strftime (buf, sizeof (buf), "%a, %d %b %Y %H:%M:%S GMT", tm);
    add_ohdr ("Date", buf);
  }
  TRACE9 ("Sending connection headers");
  ws << oheaders;
  ws << "\r\n"; 
  response_sent = true;
}

//-----------------------------------------------------------------------------
/*!
  \class server
  \ingroup sockets

  This class is derived from mlib::tcpserver class and implements a basic HTTP
  server.
  
  After construction, the main server thread has to be started by calling start()
  function. When started, the server binds to the listening socket and creates
  new \ref mlib::http::connection "http::connection" objects for each incoming
  client. All the protocol is then handled by the connection  (or derived) 
  object.

  A HTTP server can be integrated to an application by adding specific handlers
  and user variables. Handlers are user functions called in response to client
  requests. There are two types of handlers:
  - _global handlers_ registered using the add_handler() function. These are
  called by the client connection thread immediately after validating a client
  request.
  - _post handlers_ registered using the add_post_handler() function. These are
  called only for POST or PUT requests.
  
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

  - document root is current folder
  - default url is HTTP_DEFAULT
  - server name is HTTP_SERVER_NAME
*/
server::server (unsigned short port, unsigned int maxconn)
  : tcpserver (port, HTTP_SERVER_NAME, maxconn)
  , root (std::filesystem::current_path())
  , defuri (HTTP_DEFAULT_URI)
  , timeout (HTTP_TIMEOUT)
{
  name (HTTP_SERVER_NAME);
}

/*!
  Destructor
*/
server::~server ()
{
}

/*!
  Change server name.
  \param name_ new server name string

  This string is returned in the "Server" HTTP header.
*/
void server::name (const std::string& name_)
{
  if (!name_.empty())
    add_ohdr ("Server", name_);
  else
    remove_ohdr ("Server");
  thread::name (name_);
}

/*!
  Create an new mlib::http::connection object in response to a new client
  connection request.

  Derived classes can override this function to return objects derived from
  mlib::http::connection class.
*/
connection* server::make_thread (sock& connection)
{
  return new http::connection (connection, *this);
}

/*!
  Add or modify a server response header.
  \param hdr      Header name
  \param value    Header value

  Server response headers are always sent as part of the HTTP answer.
  In addition each connection object can add it's own headers.
*/
void server::add_ohdr (const std::string& hdr, const std::string& value)
{
  lock l (hdr_lock);
  out_headers[hdr] = value;
}

/*!
  Remove a server response header.
  \param hdr    Header name
*/
void server::remove_ohdr (const std::string& hdr)
{
  lock l (hdr_lock);
  auto idx = out_headers.find (hdr);
  if (idx != out_headers.end ())
    out_headers.erase (idx);
}

/*!
  \param uri    URI address
  \param func   handler function
  \param info   handler specific information

  When a HTTP client requests the URI, the connection object that services the
  client connection will invoke the user-defined handler function passing the
  connection object and the handler specific information.

  Example:
\code{.cpp}
  int say_hello (http::connection& client, void *)
  {
    client.serve_buffer ("Hello world!");
    return HTTP_OK;
  }
  //...
  http::server my_server(8080);

  my_server.add_handler ("/greeting", say_hello, nullptr);
\endcode
  In response to a request to http://localhost:8080/greeting, the server will
  display the greeting message.
  
  \note These handlers are invoked irrespective of the HTTP method (GET, 
  POST, DELETE, etc.).
*/
void server::add_handler (const std::string& uri, uri_handler func, void* info)
{
  handle_info hi {func, info};
  handlers.emplace (uri, hi);
}

/*!
  Add or modify an POST handler function.
  \param tgt_path    URI address
  \param func   handler function
  \param info   handler specific information

  These handlers work just like the handlers defined using the add_handler()
  function, except that they are invoked only for POST or PUT requests.
*/
void server::add_post_handler (const std::string& uri, uri_handler func, void* info)
{
  handle_info hi {func, info};
  post_handlers.emplace (uri, hi);
}

/*!
  Add/change content of table matching MIME types to file extensions
  \param  ext   filename extension
  \param  type  MIME type
  \param  shtml true if SSI processing should be enabled for this file type
*/
void server::add_mime_type (const std::string& ext, const std::string& type, bool shtml)
{
  smime t{type, shtml};
  knowntypes[ext] = t;
}

/*!
  Remove a file type from the MIME type table
  \param ext    filename extension
*/
void server::delete_mime_type (const std::string& ext)
{
  auto ptr = knowntypes.find (ext);
  if (ptr != knowntypes.end ())
    knowntypes.erase (ptr);
}

/*!
  Add a new access realm. Realms are assigned to specific tgt_path paths and their
  access can be restricted to specified users.
  \param realm    protection realm
  \param tgt_path      starting path
*/
void server::add_realm (const char* realm, const char* uri)
{
  string s_uri = (*uri == '/') ? uri : string ("/") + uri;
  realms[realm] = s_uri;
}

/*!
  Add a new user to a relm or modifies password for an existing user.
  \param realm access realm
  \param username user name
  \param pwd user password
  \return _true_ if successful or _false_ if realm doesn't exist
*/
bool server::add_user (const char* realm, const char* username, const char* pwd)
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

bool server::remove_user (const char* realm, const char* username)
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
  Find the realm with longest matching path that covers an URI
  \param tgt_path    URI to check
  \param realm  matching realm
  \return       true if URI is covered by a realm

*/
bool server::is_protected (const std::string& uri, std::string& realm)
{
  size_t len = 0;
  auto it = realms.begin ();
  while (it != realms.end ())
  {
    size_t n = match (uri, it->second.c_str ());

    if ((n == uri.length () || uri[n] == '/') // complete path segment
     && n == it->second.length () //matching whole realm path
     && n > len)                  //longest matching path
    {
      len = n;
      realm = it->first;
    }
    it++;
  }
  return (len > 0);
}

/*!
  Verify user credentials for a realm.
  \return     true if user is authorized for the realm
*/
bool server::authenticate (const std::string& realm, const std::string& user, const std::string& pwd)
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

bool server::locate_handler (const std::string& res, handle_info** ptr)
{
  auto h = handlers.end ();
  size_t len = 0;
  auto crt = handlers.begin ();

  while (crt != handlers.end ())
  {
    size_t n = match (res, crt->first);

    if ((n == crt->first.size ())                // matches whole handler URI
        && (n == res.length () || res[n] == '/') // complete URI or path segment
        && (n > len))                            // longer than previous match
    {
      len = n;
      h = crt;
    }
    crt++;
  }
  if (h != handlers.end ())
  {
    if (ptr)
      *ptr = &h->second;
    return true;
  }

  return false;
}
  /*!
  Invoke a user defined URI handler 
  \param  client connection thread

  \return the result of calling the user handler or HTTP_NO_HANDLER if there
    is no handler set for the URI.

*/
int server::invoke_handler (connection& client)
{
  auto uri = client.get_path ();
  int ret = HTTP_NO_HANDLER;
  handle_info* hinfo;
  if (locate_handler (client.path_, &hinfo))
  {
    lock l (*hinfo->in_use);
    TRACE9 ("Invoking handler for %s", client.get_path ().c_str ());
    ret = hinfo->h (client, hinfo->nfo);
    TRACE9 ("Handler done (%d)", ret);
  }
  
  return ret;
}


/*!
  Invoke a user defined handler in response to a POST or PUT request
  \param  client connection thread

  \return the result of calling the user handler or 0 if there is no handler
          set for the URI.

*/
int server::invoke_post_handler (connection& client)
{
  int ret = 0;
  auto idx = post_handlers.find (client.get_path ());
  if (idx != post_handlers.end ())
  {
    lock l (*idx->second.in_use);
    TRACE9 ("Invoking handler for %s", idx->first.c_str ());
    ret = idx->second.h (client, idx->second.nfo);
    TRACE9 ("Handler done (%d)", ret);
  }

  return ret;
}

/*!
  Maps a local file path to an URI resource path.
  \param  tgt_path     URI resource path
  \param  path    mapped local file path

  The mapping is always relative to server's root folder (`docroot`).
*/
void server::add_alias (const std::string& uri, const std::string& path)
{
  aliases[uri] = path;
}

/*!
  Retrieve the local file path mapped to a resource
  \param  res    resource path
  \param  path   local path

  \return true if resource was remapped.

  After processing the alias table, any part of the original resource that was
  not mapped is appended to the resulting path. For instance if the alias table
  contains an entry mapping "doc" to "documentation" and docroot is set as
  "c:\\local_folder\\", a resource like "/doc/project1/filename.html" will be
  mapped to "c:\\local_folder\\documentation\\project1\\filename.html".

  Note that mapping is only lexical. The function does not check if the resource
  exists.

  Remapping is "greedy". If the alias table contains mappings for "doc" and
  "doc/new", a resource like "/doc/new/manual.html" will use the mapping for
  "/doc/new"
*/
bool server::find_alias (const std::string& res, std::filesystem::path& path)
{
  size_t end = res.find ('/'), 
    endmap;
  std::string remap;
  while (end != string::npos)
  {
    auto pfx = aliases.find(res.substr (0, end - 1));
    if (pfx != aliases.end ())
    {
      remap = pfx->second;
      endmap = end;
    }
    end = res.find ('/', end + 1);
  }
  if (!remap.empty ())
  {
    path = root;
    path /= res.substr (endmap);
    return true;
  }
  return false;
}

/*!
  Try to map a resource to a local file.
  \param res resource to map
  \param path local file system path

  \return `true` if a local file exists and is a regular file
  \return `false` otherwise
*/
bool server::locate_resource (const std::string& res, std::filesystem::path& path)
{
  if (!find_alias (res, path))
  {
    path = docroot ();
    path.concat (res);
  }
  if (!path.has_filename ())
    path += default_uri ();

  return (std::filesystem::exists (path) && std::filesystem::is_regular_file (path));
}

/*!
  Add or modify a user variable
  \param name     variable name (the name used in SSI construct)
  \param addr     address of content
  \
  \param fmt      sprintf format string
  \param multiplier for numeric variables, resulting value is multiplied by this factor

  User variables are accessible through SSI constructs like:
  \verbatim
    <!--#echo var="name" -->
  \endverbatim

  When the page is served the SSI construct is replaced by the current value of
  the named variable, eventually multiplied by \a multiplier factor and formatted
  as text using the \a fmt string. If format string is NULL, a format appropriate
  for the variable type is used
*/
void server::add_var (const std::string& name, vtype t, const void* addr,
                     const char* fmt, double multiplier)
{
  struct var_info vi = {fmt ? fmt : string (), t, addr, multiplier};
  lock l (varlock);
  variables[name] = vi;
}

/*!
  Return the current string representation of a variable.
  \param name variable name
*/
const string server::get_var (const std::string& name)
{
  lock l (varlock);
  auto vptr = variables.find (name);
  if (vptr != variables.end ())
  {
    char buf[256];
    const auto& vi = vptr->second;
    bool nofmt = vi.fmt.empty ();
    const char* fmt = vi.fmt.c_str ();
    switch (vi.type)
    {
    case VT_CHAR:
      snprintf (buf, sizeof(buf), nofmt ? "%s" : fmt, (const char*)vi.addr);
      break;
    case VT_SHORT:
      snprintf (buf, sizeof (buf), nofmt ? "%hd" : fmt, *(const short*)vi.addr);
      break;
    case VT_USHORT:
      snprintf (buf, sizeof (buf), nofmt ? "%hu" : fmt, *(const unsigned short*)vi.addr);
      break;
    case VT_INT:
      snprintf (buf, sizeof (buf), nofmt ? "%d" : fmt, *(const int*)vi.addr);
      break;
    case VT_UINT:
      snprintf (buf, sizeof (buf), nofmt ? "%u" : fmt, *(const unsigned int*)vi.addr);
      break;
    case VT_LONG:
      snprintf (buf, sizeof (buf), nofmt ? "%ld" : fmt, *(const long*)vi.addr);
      break;
    case VT_ULONG:
      snprintf (buf, sizeof (buf), nofmt ? "%lu" : fmt, *(const unsigned long*)vi.addr);
      break;
    case VT_FLOAT:
      snprintf (buf, sizeof (buf), nofmt ? "%f" : fmt,
                *(const float*)vi.addr * (float)vi.multiplier);
      break;
    case VT_DOUBLE:
      snprintf (buf, sizeof (buf), nofmt ? "%lf" : fmt, 
                *(const double*)vi.addr * vi.multiplier);
      break;
    case VT_STRING:
      return *(const std::string*)vi.addr;
      break;

    default: //should never happen
      buf[0] = 0;
    }
    return buf;
  }
  else
    return "none";
}

/*!
  Guess MIME type of a file and if SSI replacement should be enabled based on
  file extension.
  \param fn file name
  \param shtml return true if SSI replacement should be enabled for this file type

  \return MIME type
*/
const std::string& server::guess_mimetype (const std::filesystem::path& fn, bool& shtml)
{
  if (fn.has_extension())
  {
    auto ext = fn.extension ().string ().substr(1); //extension starts with '.';
    auto ptr = knowntypes.find (ext);
    if (ptr != knowntypes.end ())
    {
      shtml = ptr->second.shtml;
      return ptr->second.type;
    }
  }
  shtml = false;
  return knowntypes[0].type;
}


/*!
  Return number of matching characters at beginning of str1 and str2.
  Comparison is case insensitive.
*/
size_t match (const std::string& str1, const std::string& str2)
{
  size_t n = 0;
  size_t len = min (str1.size (), str2.size ());
  while (n < len && tolower (str1[n]) == tolower (str2[n]))
    n++;
  return n;
}

} // namespace mlib::http


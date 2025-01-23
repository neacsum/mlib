/*!
  \file httpd.cpp Implementation of httpd and http::connection classes

  (c) Mircea Neacsu 2007-2025. All rights reserved.
*/
#include <mlib/mlib.h>
#pragma hdrstop

#include <utf8/utf8.h>
#include <utils.h>

using namespace std;
using namespace mlib;

// Defaults
#define HTTP_DEFAULT_URI "index.html"      //!< Default URL
#define HTTPD_SERVER_NAME "MLIB_HTTP 2.0"   //!< Default server name

/// Timeout interval while waiting for a client response
#define HTTPD_TIMEOUT 30

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
  \class http::connection
  \ingroup  sockets

  This is the thread created by the http::server object in response to a new
  connection request.

  After construction, the thread is started automatically by the server and it
  begins listening to its connection socket and process HTTP requests. When the
  HTTP client closes the connection the thread is stopped and destructed.

  Users can create derived classes but the parent server must also be a class
  derived from http::server with an overridden http::server::make_thread function.

  The request processing cycle starts with the receiving a client request,
  validating, processing it and sending back the reply. The processing part
  implies either calling a user handler function, if one was registered for the
  URI, or serving a file.

  Most of the public functions are designed for the benefit of user handler
  functions. The handler function is called before sending any reply to
  the HTTP client.
*/

/*!
  Protected constructor used by http::server class to create a new connection
  thread

  \param  socket  connecting socket
  \param  server  parent server object

  As it has only a protected constructor it is not possible for users of this
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
{}

/*!
  Thread run loop.

  The loop collects the client request, parses, validates and processes it.
*/
void connection::run ()
{
  ws->recvtimeout (HTTPD_TIMEOUT);
#if defined(MLIB_TRACE)
  inaddr addr;
  ws->peer (addr);
  TRACE8 ("http::connection::run - Connection from %s", addr.hostname ().c_str ());
#endif
  try
  {
    while (1)
    {
      std::string request;
      content_len = -1;
      iheaders.clear ();
      oheaders.clear ();
      qparams.clear ();
      bparams.clear ();
      body.clear ();
      query_parsed = body_parsed = response_sent = false;

      // Accumulate HTTP request
      bool eol_seen = false;
      while (request.size () < HTTPD_MAX_HEADER)
      {
        // wait for chars.
        char ch = ws.get ();

        if (ws.eof ())
          return; // client closed

        if (!ws.good ())
        {
#if defined(MLIB_TRACE)
          inaddr addr;
          ws->peer (addr);
          TRACE2 ("http::connection::run - timeout peer %s", addr.hostname ().c_str ());
#endif
          respond (408);
          return;
        }

        if (ch == '\r')
          continue;

        request.push_back (ch);
        if (ch == '\n')
        {
          if (eol_seen)
            break; //end of headers
          else
            eol_seen = true;
        }
        else
          eol_seen = false;
      }

      if (request.size () == HTTPD_MAX_HEADER)
      {
#if defined(MLIB_TRACE)
        ws->peer (addr);
        TRACE2 ("http::connection::run - Request too long (%d) peer %s", request.size (),
                addr.hostname ().c_str ());
#endif
        respond (413);
        return;
      }
      if (!parse_request (request) || !parse_headers ())
      {
        respond (400);
        return;
      }

      int auth_stat = do_auth ();
      if (auth_stat <= 0)
        return; // bad auth - response sent by do_auth

      // set timeout to whatever client wants
      if (has_ihdr ("Keep-Alive"))
      {
        int kaval = min (stoi (get_ihdr ("Keep-Alive")), 600);
        ws->recvtimeout (kaval);
      }

      if (method == "POST" || method == "PUT")
      {
        if (!has_ihdr ("Content-Length"))
        {
          respond (400); // must have "Content-Length" header
          return;
        }
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
          ws->peer (addr);
          TRACE2 ("http::connection::run - Invalid content length (%s) peer %s cause %s",
                  cl.c_str (),
                  addr.hostname ().c_str (), x.what());
          body.clear ();
          respond (400); // invalid "Content-Length" header
          return;
        }
      }
      process_valid_request ();

      // should close connection?
      if (!has_ohdr ("Content-Length") // could not generate content length (shtml pages)
       || (has_ihdr("Connection") && get_ihdr ("Connection") == "Close")  // client wants to close
       || (has_ohdr ("Connection") && (get_ohdr ("Connection") == "Close"))) // server wants to close
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

void connection::term ()
{
#if defined(MLIB_TRACE)
  inaddr addr;
  ws->peer (addr);
  TRACE8 ("http::connection::term - Closed connection to %s", addr.hostname ().c_str ());
#endif
  ws.flush ();
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
int connection::do_auth ()
{
  string realm;
  if (!parent.is_protected (uri, realm))
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
  // check if there is a handler registered for this URI
  if (parent.invoke_handler (*this))
  {
    TRACE9 ("handler done");
    return;
  }

  if (method == "GET" || method == "HEAD")
  {
    // try to see if we can serve a file
    std::filesystem::path fullpath;
    if (!parent.find_alias (uri, fullpath))
    {
      fullpath = parent.docroot ();
      fullpath.concat (uri);
    }
    if (!fullpath.has_filename ())
      fullpath += parent.default_uri ();

    if (std::filesystem::exists (fullpath) && std::filesystem::is_regular_file (fullpath))
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
  else if (method == "POST" || method == "PUT")
  {
    if (!parent.invoke_post_handler (*this))
      respond (204);
  }
  else
    respond (400);
}

/*!
  Sends a 404 (page not found) response
*/
void connection::serve404 (const char* text)
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


bool connection::parse_headers ()
{
  auto ptr = headers.begin();
  string key, val;
  int fsm_state = 0;
  iheaders.clear ();
  do
  {
    switch (fsm_state)
    {
    case 0: //initial header name character
      if (ptr == headers.end ())
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
  } while (ptr != headers.end () && fsm_state >= 0);
  
  if (fsm_state != 0)
    return false;

  if (http_version == "HTTP/1.1" && !has_ihdr ("Host"))
    return false;

  return true;
}

bool connection::parse_body ()
{
  // check if we have what to parse
  if (body.empty() || !has_ihdr ("Content-Type") || body_parsed)
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
  parse_urlparams (query, qparams);
  query_parsed = true;
}


/*!
  Send the content of a file, processing any SSI directives.

  \param  fname file name

  \return 0 if successful or one of the following values:
  \retval -1 socket write failure
  \retval -2 file open error

  As we don't know the size of the response, the file is sent without a
  "Content-Length" header.
*/
int connection::serve_shtml (const std::filesystem::path& fname)
{
  FILE* fn = utf8::fopen (fname.string (), "rS");
  if (fn == NULL)
    return -2;

  respond (200);
  if (get_method () == "HEAD")
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
  \retval HTTPD_ERR_WRITE  - socket write failure

  The buffer is preceded by "Content-Length" header.
*/
int connection::serve_buffer (const BYTE* src_buff, size_t sz)
{
  int ret = HTTPD_OK;

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
int connection::serve_buffer (const std::string& str)
{
  return serve_buffer ((const BYTE*)str.data (), str.size ());
}

/*!
  Send the content of a file.

  \param  fname file name

  \return 0 if successful or one of the following codes:
  \retval HTTPD_ERR_WRITE  - socket write failure
  \retval HTTPD_ERR_FOPEN  - file open failure
  \retval HTTPD_ERR_FREAD  - file read failure

  The file is preceded by "Content-Length" header.
*/
int connection::serve_file (const std::filesystem::path& fname)
{
  int ret = HTTPD_OK;
  FILE* fin = utf8::fopen (fname.u8string (), "rbS");
  if (!fin)
  {
    TRACE2 ("File %s - open error", fname.u8string ());
    return HTTPD_ERR_FOPEN;
  }
  // find file size
  unsigned int len;
  fseek (fin, 0, SEEK_END);
  len = ftell (fin);
  fseek (fin, 0, SEEK_SET);
  TRACE8 ("http::connection::serve_file - File %s size %d", fname.u8string (), len);
  char flen[30];
  sprintf (flen, "%d", len);
  add_ohdr ("Content-Length", flen);
  respond (200);
  if (get_method() != "HEAD")
  {
    char buf[1024];
    int cnt = 1;
    while (cnt)
    {
      cnt = (int)fread (buf, 1, sizeof (buf), fin);
      if (!cnt && ferror (fin))
      {
        TRACE2 ("File %s - file read error %d", fname.u8string (), ferror (fin));
        ret = HTTPD_ERR_FREAD;
      }
      ws.write (buf, cnt);
      if (!ws.good ())
      {
        TRACE2 ("File %s - socket write failure", fname.u8string ());
        ret = HTTPD_ERR_WRITE;
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
  auto preq = req.begin();

  // method
  while (preq != req.end() && *preq != ' ' && *preq != '\n')
    preq++;
  if (*preq != ' ')
    return false;
  method = string (req.begin (), preq);
  str_upper (method);

  // SP
  while (*++preq == ' ')
    ;

  // Request-URI
  auto pbeg = preq;
  while (preq != req.end() && *preq != ' ' && *preq != '\n')
    ++preq;
  if (*preq != ' ')
    return false;
  
  // URI must be in origin-form: absolute-path ["?" query] ["#" fragment] 
  if (*pbeg != '/')
    return false; 

  uri = string (pbeg, preq);

  // SP
  while (*++preq == ' ')
    ;

  // HTTP-Version
  pbeg = preq;
  while (preq != req.end () && *preq != '\n')
    ++preq;
  if (*preq != '\n')
    return false;
  http_version = string (pbeg, preq);

  // headers start on next line
  headers = string(preq+1, req.end()-1);

  // separate query part
  auto qbeg= uri.find ('?');
  if (qbeg != string::npos)
  {
    query = uri.substr (qbeg + 1);
    auto qend = query.find ('#');
    if (qend != string::npos)
      query.erase (qend);
    uri.erase (qbeg);
  }

  return true;
}


/*!
  Generate a HTTP redirect response to a new uri.

  \param  uri   redirected uri
  \param  code  redirect code
*/
void connection::redirect (const std::string& uri, unsigned int code)
{
  add_ohdr ("Location", uri);
  respond (code);
  if (code == 303 && get_method () == "HEAD")
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
void connection::respond (unsigned int code, const char* reason)
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
  
  if (response_sent)
    return; // already sent response

  int ic = 0;
  while (respcodes[ic].code && respcodes[ic].code != code)
    ic++;
  ws << "HTTP/1.1 " << code;
  if (reason)
    ws << " " << reason;
  else if (respcodes[ic].code)
    ws << " " << respcodes[ic].text;
  ws << "\r\n";

  // output server headers
  lock l (parent.hdr_lock);
  idx = parent.out_headers.begin ();
  while (idx != parent.out_headers.end ())
  {
    ws << idx->first << ": " << idx->second << "\r\n";
    idx++;
  }
  response_sent = true;
  // followed by our headers
  TRACE9 ("Sending connection headers");
  idx = oheaders.begin ();
  while (idx != oheaders.end ())
  {
    ws << idx->first << ": " << idx->second << "\r\n";
    idx++;
  }
  ws << "\r\n";
}


/*!
  Send first part of a multi-part response.
  \param part_type value of the 'Content-Type' header
  \param bound part boundary string
*/
void connection::respond_part (const char* part_type, const char* bound)
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
void connection::respond_next (bool last)
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
  \class server
  \ingroup sockets

  This class is derived from tcpserver class and implements a basic HTTP server.
  After construction, the main server thread has to be started by calling start()
  function. When started, the server binds to the listening socket and creates
  new http::connection objects for each incoming client. All the protocol is then
  handled by the http::connection (or derived) class.

  A HTTP server can be integrated to an application by adding specific url handlers
  and user variables. URL handlers are registered by calling the add_handler()
  function.

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
  : tcpserver (port, HTTPD_SERVER_NAME, maxconn)
  , root (std::filesystem::current_path())
  , defuri (HTTP_DEFAULT_URI)
{
}

/*!
  Destructor
*/
server::~server ()
{}

/*!
  Change server name.
  \param nam new server name string

  This string is returned in the "Server" HTTP header.
*/
void server::name (const char* nam)
{
  if (nam)
    add_ohdr ("Server", nam);
  else
    remove_ohdr ("Server");
  thread::name (nam);
}

/*!
  Create an new http::connection object in response to a new client connection
  request.

  Derived classes can override this function to return objects derived from
  http::connection class.
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
  Add or modify an URI handler function.
  \param uri    URI address
  \param func   handler function
  \param info   handler specific information

  When a HTTP client requests the URI, the connection object that services the
  client connection will invoke the user-defined handler function passing the
  connection object and the handler specific information.

  Note that these handlers are invoked irrespective of the HTTP method (GET, 
  POST, DELETE, etc.).
*/
void server::add_handler (const std::string& uri, uri_handler func, void* info)
{
  handle_info hi {func, info};
  handlers.emplace (uri, hi);
}

/*!
  Add or modify an POST handler function.
  \param uri    URI address
  \param func   handler function
  \param info   handler specific information

  These handlers work just like the handlers defined using the add_handler()
  function, except that they are invoked only for POST or PUT requests.
*/
void server::add_post_handler (const std::string& uri, uri_handler func, void* info)
{
  handle_info hi {func, info};
  handlers.emplace (uri, hi);
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
  Add a new access realm. Realms are assigned to specific uri paths and their
  access can be restricted to specified users.
  \param realm    protection realm
  \param uri      starting path
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
  \param uri    URI to check
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

/*!
  Invoke a user defined URI handler
  \param  client connection thread

  \return the result of calling the user handler or 0 if there is no handler
          set for the URI.

*/
int server::invoke_handler (connection& client)
{
  int ret = 0;
  auto idx = handlers.find (client.get_uri());
  if (idx != handlers.end ())
  {
    lock l (*idx->second.in_use);
    TRACE9 ("Invoking handler for %s", idx->first.c_str());
    ret = idx->second.h (client, idx->second.nfo);
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
  auto idx = post_handlers.find (client.get_uri ());
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
  \param  uri     URI resource path
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
  Set server root path
*/
void server::docroot (const std::string& path)
{
  root = std::filesystem::absolute(path);
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


#include <mlib/mlib.h>
#pragma hdrstop

#include <utils.h>

/// Parse a URL encoded string of parameters
bool parse_urlparams (const std::string& str, mlib::http::str_pairs& params)
{
  std::vector<char> vstr (str.begin (), str.end ());

  auto beg = vstr.begin();

  do
  {
    auto end = std::find (beg, vstr.end(), '&');
    auto kend = std::find (beg, end, '=');
    if (kend == end)
      return false;

    std::string key(beg, kend);
    std::string val(kend + 1, end);
    if (!url_decode (val))
      return false;
    params[key] = val;
    if (end != vstr.end ())
      beg = end + 1;
    else
      break;
  } while (1);

  return true;
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

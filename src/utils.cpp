#include <mlib/mlib.h>
#pragma hdrstop

#include <utils.h>

namespace mlib::internal {

/// Parse a URL encoded string of parameters
bool parse_urlparams (const std::string& str, mlib::http::str_pairs& params)
{
  std::vector<char> vstr (str.begin (), str.end ());

  auto beg = vstr.begin ();

  do
  {
    auto end = std::find (beg, vstr.end (), '&');
    auto kend = std::find (beg, end, '=');
    if (kend == end)
      return false;

    std::string key (beg, kend);
    if (!url_decode (key))
      return false;
    std::string val (kend + 1, end);
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
  Decodes URL-encoded data.

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

} // namespace mlib::internal
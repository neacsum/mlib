#pragma once
// Various utility functions for internal use only

#include <string>
#include <algorithm>

inline void str_lower (std::string& s)
{
  std::transform (s.begin (), s.end (), s.begin (),
                  [] (char c) -> char { return tolower (c); });
}

inline void str_upper (std::string& s)
{
  std::transform (s.begin (), s.end (), s.begin (), 
    [] (char c) -> char { return toupper (c); });
}

bool url_decode (std::string& s);
bool parse_urlparams (const std::string& par_str, mlib::http::str_pairs& params);

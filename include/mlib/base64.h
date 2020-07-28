/*!
  \file   base64.h Definition of Base64 encoding/decoding functions

  (c) Mircea Neacsu 2017
*/
#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

#include <string>

namespace mlib {

size_t base64dec (const char *in, void *out);
size_t base64enc (const void *in, char *out, size_t ilen);

std::string base64enc (const std::string& in);
std::string base64dec (const std::string& in);

};

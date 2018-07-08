#pragma once

#if __has_include ("defs.h")
#include "defs.h"
#endif

#include <string>

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

size_t base64dec (const char *in, void *out);
size_t base64enc (const void *in, char *out, size_t ilen);

std::string base64enc (const std::string& in);
std::string base64dec (const std::string& in);

#ifdef MLIBSPACE
};
#endif

#pragma once
#include "defs.h"

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

size_t base64dec (const char *in, void *out);
size_t base64enc (const void *in, char *out, size_t ilen);

#ifdef MLIBSPACE
};
#endif

#pragma once
#include "defs.h"

namespace mlib {

size_t base64dec (const char *in, void *out);
size_t base64enc (const void *in, char *out, size_t ilen);

};

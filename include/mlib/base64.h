/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file   base64.h Definition of Base64 encoding/decoding functions
#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include <string>

namespace mlib {

size_t base64dec (const char* in, void* out);
size_t base64enc (const void* in, char* out, size_t ilen);

std::string base64enc (const std::string& in);
std::string base64dec (const std::string& in);

}; // namespace mlib

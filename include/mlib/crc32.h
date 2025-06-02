/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

/// \file crc32.h Functions to compute CRC32

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

namespace mlib {

void crc32_update (unsigned char byte, uint32_t* crc);
uint32_t crc32 (const void* block, size_t sz);

} //namespace mlib


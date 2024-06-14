#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

namespace mlib 
{

void crc32_update (BYTE byte, DWORD* crc);
DWORD crc32 (const void* block, size_t sz);

}


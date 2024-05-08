#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

void crc32_update (BYTE byte, DWORD* crc);
DWORD crc32 (const void* block, size_t sz);

#ifdef __cplusplus
}
#endif

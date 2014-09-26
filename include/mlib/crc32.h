#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void crc32_update (BYTE byte, DWORD *crc);

DWORD crc32 (void *block, size_t sz);

#ifdef  __cplusplus
}
#endif

#include <windows.h>
#include <mlib/crc32.h>
#include <utpp/utpp.h>


TEST (CRC32_QuickBrownFox)
{
  char *fox = "The quick brown fox jumps over the lazy dog";
  DWORD crc= crc32 (fox, strlen(fox));

  CHECK_EQUAL (0x414fa339, crc);
}

TEST (CRC32_Boost)
{
  //Test vector from http://www.boost.org/doc/libs/1_37_0/libs/crc/crc_test.cpp

  unsigned char  std_data[] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
  size_t const std_data_len = sizeof( std_data ) / sizeof( std_data[0] );  
  DWORD const  std_crc_32_result = 0xCBF43926;
  DWORD crc = crc32 (std_data, std_data_len);

  CHECK_EQUAL (std_crc_32_result, crc);
}
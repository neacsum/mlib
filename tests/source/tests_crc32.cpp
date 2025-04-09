#include <mlib/mlib.h>
#include <utpp/utpp.h>
#pragma hdrstop


TEST (CRC32_QuickBrownFox)
{
  const char *fox = "The quick brown fox jumps over the lazy dog";
  DWORD crc = mlib::crc32 (fox, strlen (fox));

  CHECK_EQUAL (0x414fa339, crc);
}

TEST (CRC32_Boost)
{
  //Test vector from http://www.boost.org/doc/libs/1_37_0/libs/crc/crc_test.cpp

  unsigned char  std_data[] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
  size_t const std_data_len = sizeof( std_data ) / sizeof( std_data[0] );  
  DWORD const  std_crc_32_result = 0xCBF43926;
  DWORD crc = mlib::crc32 (std_data, std_data_len);

  CHECK_EQUAL (std_crc_32_result, crc);
}

TEST (MD5_QuickBrownFox)
{
  //Test vector from https://www.febooti.com/products/filetweak/members/hash-and-crc/test-vectors/

  const char* fox = "The quick brown fox jumps over the lazy dog";
  const char* expected = "9e107d9d372bb6826bd81d3542a419d6";
  mlib::md5 hasher;
  hasher.append ((const unsigned char *)fox, strlen (fox));
  unsigned char digest[16];
  char result[33];
  hasher.finish (digest);
  mlib::binhex (result, digest, sizeof (digest));
  CHECK_EQUAL (expected, result);
}
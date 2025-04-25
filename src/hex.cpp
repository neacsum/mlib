/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This is part of MLIB project. See LICENSE file for full license terms.
*/

#include <mlib/mlib.h>
#pragma hdrstop

#ifndef _MSC_VER
#define LOWORD(l) ((WORD)(l))
#define HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w) ((BYTE)(w))
#define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#endif

namespace mlib {

/*!
  \param str output string
  \param bin input binary data
  \param sz size of binary data

  The output string is null-terminated. It must be at least 2*`sz`+1 bytes.
*/
void binhex (char* str, const unsigned char* bin, size_t sz)
{
  for (size_t i=0; i<sz; i++)
  {
    bytehex (str, *bin++);
    str += 2;
  }
  *str = 0;
}

/*!
  \param str output string. Must be at least 3 bytes long
  \param bin input binary value

  The output string is null-terminated.
*/
void bytehex (char* str, unsigned char bin)
{
  unsigned char nibble;

  nibble = bin >> 4;
  *str++ = (nibble > 9)? nibble-10+'a' : nibble+'0';
  nibble = bin & 0x0f;
  *str++ = (nibble > 9)? nibble-10+'a' : nibble+'0';
  *str = 0;
}

/*!
  \param str output string. Must be at least 5 bytes long
  \param bin input binary value

  The output string is null-terminated.
*/
void shorthex (char* str, unsigned short bin)
{
  bytehex (str, HIBYTE(bin));
  bytehex (str+2, LOBYTE(bin));
  str[4] = 0;
}

/*!
  \param str output string. Must be at least 9 bytes long
  \param bin input binary value

  The output string is null-terminated.
*/
void longhex (char *str, unsigned long bin)
{
  bytehex (str, HIBYTE(HIWORD(bin)));
  bytehex (str+2, LOBYTE(HIWORD(bin)));
  bytehex (str+4, HIBYTE(LOWORD(bin)));
  bytehex (str+6, LOBYTE(LOWORD(bin)));
  str[8] = 0;
}

/*!
  \param bin resulting byte value
  \param str input string of hex digits

  \return `true` if conversion was successful, `false` otherwise
*/
bool hexbyte (unsigned char& bin, const char* str)
{
  unsigned char d1, d2;

  //first digit
  if (*str >= '0' && *str <='9')
    d1 = *str++ - '0'; 
  else if (*str >= 'A' && *str <= 'F')
    d1 = *str++ - 'A'+10;
  else if (*str >= 'a' && *str <= 'f')
    d1 = *str++ - 'a'+10;
  else
     return false;

  //2nd digit
  if (*str >= '0' && *str <='9')
    d2 = *str - '0'; 
  else if (*str >= 'A' && *str <= 'F')
    d2 = *str - 'A'+10;
  else if (*str >= 'a' && *str <= 'f')
    d2 = *str - 'a'+10;
  else
     return false;

  bin = (d1 << 4) | d2;
  return true;
}

/*!
  \param bin resulting short integer value
  \param str input string of hex digits

  \return `true` if conversion was successful, `false` otherwise
*/
bool hexshort (unsigned short& bin, const char* str)
{
  unsigned char lo, hi;
  if (strlen (str) < 4)
    return false;

  if (hexbyte (hi, str) && hexbyte (lo, str + 2))
  {
    bin = (hi << 8) | lo;
    return true;
  }
  return false;    
}

/*!
  \param bin resulting long integer value
  \param str input string of hex digits

  \return `true` if conversion was successful, `false` otherwise
*/
bool hexlong (unsigned long& bin, const char* str)
{
  unsigned short lo, hi;
  if (strlen (str) < 8)
    return false;

  if (hexshort (hi, str) && hexshort (lo, str + 4))
  {
    bin = (hi << 16) | lo;
    return true;
  }
  return false;
}

/*!
  \param bin resulting binary data
  \param str input hex string
  \param sz maximum size of binary data

  \return number of bytes converted
*/
size_t hexbin (unsigned char *bin, const char *str, size_t sz)
{
  size_t cnt = 0;

  while (sz && hexbyte (*bin, str))
  {
    str += 2;
    bin++;
    sz--;
    cnt++;
  }

  return cnt;
}

} //namespace mlib
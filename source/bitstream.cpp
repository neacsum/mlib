/*!
  \file BITSTREAM.CPP Implementation of bitstream class

  (c) Mircea Neacsu 2017
*/
#include <mlib/bitstream.h>

using namespace std;

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/*!
  \class bitstream
  
  This class allows reading and writing of bit fields from or to a byte stream.
  It allows for different byte encodings using the protected encode/decode functions.
  
  \remarks Maximum field width is limited to 32 bits.
*/

/*!
  \param str    Underlining iostream
  \param pack   Number of bits per byte
*/
bitstream::bitstream (std::iostream& str, unsigned int pack) 
  : packing (pack)
  , s(str)
  , nbits (0)
  , buffer(0)
{
}


/*!
  Read a single bit from stream. If there are no more bits available in current
  byte tries to get a new byte and decodes it using #decode function.

  \return Bit value

  \remarks    All bits read after stream reaches EOF are 0.
*/
bool bitstream::read ()
{
  if (nbits == 0)
  {
    char next;
    s >> next;
    if (s.eof () || s.bad ())
      return false;
    decode (buffer, next);
    nbits = packing;
  }
  bool val = buffer & (1<<(packing-1));
  nbits--;
  buffer <<= 1;
  return val;
}

/*!
  If it has accumulated enough bits calls #encode function to encode them and
  writes a new byte to underlining stream.
  
  \param val    Bit value to write

*/
void bitstream::write (int val)
{
  if (nbits == packing)
  {
    char next;
    encode (buffer, next);
    s << next;
    buffer = 0;
    nbits = 0;
  }
  buffer <<= 1;
  if (val)
    buffer |= 1;
  nbits++;
}

/*!
  If there are any leftover bits, encode them in one character and
  write them out to stream.
*/
void bitstream::flush ()
{
  if (nbits)
  {
    char next;
    encode (buffer, next);
    s << next;
    buffer = 0;
    nbits = 0;
  }
  s.flush ();
}

/*!
  Invokes #write function repeatedly to write each bit.

  \param val    Value to write (0 to 2<sup>sz</sup>-1 ) 
  \param sz     Width of bit field  
*/
void bitstream::mwrite (int val, unsigned int sz)
{
  unsigned int mask = 1<<(sz-1);
  for (unsigned int i=0; i<sz; i++)
  {
    write (val & mask);
    mask >>= 1;
  }
}

/*!
  Invokes #read function repeatedly to read all bits.

  \param sz         Width of bit field  
  \param sign       "true" if bit field should be interpreted as a signed value.  
  \return           value (0 to 2<sup>sz</sup>-1 or -2<sup>sz-1</sup> to 2<sup>sz-1</sup>)

*/
int bitstream::mread (unsigned int sz, bool sign)
{
  int val = read()? (sign?-1:1) : 0;
  for (unsigned int i = 1; i<sz; i++)
  {
    val <<= 1;
    if (read ())
      val |= 1;
  }

  return val;
}


#ifdef MLIBSPACE
};
#endif


/*!
  \file BITSTREAM.H - Definition of bitstream class.
*/
#pragma once
#include <iostream>

#if __has_include ("defs.h")
#include "defs.h"
#endif

#ifdef MLIBSPACE
namespace MLIBSPACE {
#endif

/// Read and write bit fields
class bitstream
{
public:
  bitstream (std::iostream& str, unsigned int pack);

  bool read ();                     ///< Read the next bit
  void write (int val);             ///< Write a bit to stream

  ///Read a variable number of bits
  int mread (unsigned int sz, bool is_signed = false);

  ///Write a variable number of bits.
  void mwrite (int val, unsigned int sz);

protected:
  const unsigned int packing;
  std::iostream& s;

  ///Encode bit field in a byte
  virtual void enc (unsigned char bits, char &chr);

  ///Decode bit field from a byte
  virtual void dec (unsigned char& bits, char chr);

private:
  unsigned char buffer;
  int nbits;
};

/*!
  The default implementation packs the bit field in the LSB of byte.
  \param bits     bit field to encode
  \param chr      encoded byte
*/
inline void
bitstream::enc (unsigned char bits, char &chr)
{
  chr = (bits & (1 << packing) - 1);
}

/*!
  The default implementation unpacks the bit field from the LSB of byte.
  \param bits     resulting bit field
  \param chr      encoded byte
*/
inline void
bitstream::dec (unsigned char& bits, char chr)
{
  bits = (chr & (1 << packing) - 1);
}

#ifdef MLIBSPACE
};
#endif

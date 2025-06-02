/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

/// \file bitstream.h Definition of mlib::bitstream class.

#pragma once
#include <iostream>

#if __has_include("defs.h")
#include "defs.h"
#endif

namespace mlib {

/// Read and write bit fields
class bitstream
{
public:
  bitstream (std::iostream& str, unsigned int pack);

  /// Read the next bit
  bool read ();

  /// Read a number of bits
  int read (size_t sz, bool is_signed = false);

  /// Write a bit to stream
  void write (bool val);

  /// Write a number of bits.
  void write (int val, size_t sz);

  void flush ();

protected:
  const unsigned int packing; //!< number of used bits per byte
  std::iostream& s;           //!< underlining (byte) stream

  /// Encode bit field in a byte
  virtual char encode (unsigned char bits);

  /// Decode bit field from a byte
  virtual unsigned char decode (char chr);

private:
  unsigned char buffer;
  int nbits;
};

/*!
  The default implementation packs the bit field in the LSB of byte.
  \param bits     bit field to encode
  \return         encoded byte
*/
inline 
char bitstream::encode (unsigned char bits)
{
  return (bits & (1 << packing) - 1);
}

/*!
  The default implementation unpacks the bit field from the LSB of byte.
  \param chr      encoded byte
  \return         resulting bit field
*/
inline unsigned char bitstream::decode (char chr)
{
  return (chr & (1 << packing) - 1);
}

}; // namespace mlib

#pragma once
/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This is part of MLIB project. See LICENSE file for full license terms.
*/

namespace mlib {

/// convert a string of hex digits to binary
size_t hexbin (unsigned char* dst, const char* src, size_t sz);

/// convert binary data to a string of hex digits
void binhex (char* dst, const unsigned char* src, size_t sz);

/// convert one byte to two hex digits
void bytehex (char* dst, unsigned char bin);

/// convert a short integer to 4 hex digits
void shorthex (char* dst, unsigned short bin);

/// convert a long integer to 8 hex digits
void longhex (char* dst, unsigned long bin);

/// convert two ASCII hex digits to binary
bool hexbyte (unsigned char& dst, const char* src);

/// convert four ASCII hex digits to binary
bool hexshort (unsigned short& dst, const char* src);

/// convert eight ASCII hex digits to binary
bool hexlong (unsigned long& dst, const char* src);

} //namespace mlib


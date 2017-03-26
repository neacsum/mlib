#pragma once

// Standard defines documented here: http://predef.sourceforge.net

#if defined(_MSC_VER)
  #pragma warning(disable:4127) // conditional expression is constant
  #pragma warning(disable:4702) // unreachable code
  #pragma warning(disable:4722) // destructor never returns, potential memory leak

#endif


// by default, MemoryOutStream is implemented in terms of std::ostringstream, which can be expensive.
// uncomment this line to use the custom MemoryOutStream (no deps on std::ostringstream).

//#define UNITTEST_USE_CUSTOM_STREAMS

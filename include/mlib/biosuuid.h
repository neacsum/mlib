/*!
  \file biosuuid.h Declaration of biosuuid() function
*/

#pragma once

namespace mlib {

/// Retrieve BIOS UUUID.
bool biosuuid (unsigned char* uuid);

} // namespace mlib
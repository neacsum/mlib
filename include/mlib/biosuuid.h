/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///   \file biosuuid.h Declaration mlib::biosuuid() function

#pragma once

namespace mlib {

/// Retrieve BIOS UUUID.
bool biosuuid (unsigned char* uuid);

} // namespace mlib
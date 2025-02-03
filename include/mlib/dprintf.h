/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This is part of MLIB project. See LICENSE file for full license terms.
*/

#pragma once

/// \file dprintf.h Definition of dprintf () function

#define MAX_DPRINTF_CHARS 1024 //!< maximum message size

bool dprintf (const char* fmt, ...);

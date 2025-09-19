/*
  Copyright (c) Mircea Neacsu (2014-2025) Licensed under MIT License.
  This file is part of MLIB project. See LICENSE file for full license terms.
*/

///  \file mlib.h Uber-include file for mlib library

#pragma once

#if __has_include("defs.h")
#include "defs.h"
#endif

#include "safe_winsock.h"

#include "base64.h"
#include "bitstream.h"
#include "border.h"
#include "convert.h"
#include "crc32.h"
#include "dprintf.h"
#include "errorcode.h"
#include "hex.h"
#include "ipow.h"
#include "json.h"
#include "md5.h"
#include "nmea.h"
#include "options.h"
#include "point.h"
#include "poly.h"
#include "ringbuf.h"
#include "rotmat.h"
#include "statpars.h"
#include "stopwatch.h"
#include "trace.h"
#include "tvops.h"

// sqlite3 wrappers needs SQLITE3 headers
#if __has_include("sqlite3/sqlite3.h")
#include "sqlitepp.h"
#endif

// Windows specific stuff
#ifdef _MSC_VER

#include "asset.h"
#include "basename.h"
#include "chull.h"
#include "firewall.h"
#include "http.h"
#include "jbridge.h"
#include "log.h"
#include "mutex.h"
#include "semaphore.h"
#include "serenum.h"
#include "shmem.h"
#include "sock.h"
#include "sockbuf.h"
#include "sockstream.h"
#include "syncque.h"
#include "tvops.h"
#include "wtimer.h"

#if !defined(NODEFAULTLIB)
#pragma comment(lib, "mlib.lib")
#endif

#endif

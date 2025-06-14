#pragma once
/*!
  \file mlib.h Uber-include file for mlib library

  (c) Mircea Neacsu 2019
*/

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
#include "sqlitepp.h"
#include "statpars.h"
#include "stopwatch.h"
#include "trace.h"

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
#include "rdir.h"
#include "semaphore.h"
#include "serenum.h"
#include "shmem.h"
#include "syncque.h"
#include "tvops.h"
#include "wtimer.h"

#if !defined(NODEFAULTLIB)
#pragma comment(lib, "mlib.lib")
#endif

#endif

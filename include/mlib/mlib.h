#pragma once
/*!
  \file mlib.h Uber-include file for mlib library

  (c) Mircea Neacsu 2019
*/

#if __has_include ("defs.h")
#include "defs.h"
#endif

#include <Winsock2.h>

#include "base64.h"
#include "basename.h"
#include "bitstream.h"
#include "border.h"
#include "chull.h"
#include "convert.h"
#include "crc32.h"
#include "firewall.h"
#include "jbridge.h"
#include "log.h"
#include "md5.h"
#include "mutex.h"
#include "nmea.h"
#include "options.h"
#include "rdir.h"
#include "ringbuf.h"
#include "semaphore.h"
#include "serenum.h"
#include "shmem.h"
#include "sqlitepp.h"
#include "statpars.h"
#include "stopwatch.h"
#include "syncque.h"
#include "trace.h"
#include "wtimer.h"


target_sources(mlib PRIVATE
  geom/border.cpp
  geom/chull.cpp
  geom/rotmat.cpp
  base64.cpp
  bitstream.cpp
  convert.cpp
  crc32.cpp
  dprintf.cpp
  json.cpp
  md5.c
  nmea.cpp
  options.cpp
  sqlitepp.cpp
  statpars.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_sources(mlib PRIVATE
  asset.cpp
  basename.cpp
  biosuuid.cpp
  event.cpp
  firewall.cpp
  httpd.cpp
  inaddr.cpp
  jbridge.cpp
  log.cpp
  mutex.cpp
  rdir.cpp
  semaphore.cpp
  serenum1.cpp
  serenum2.cpp
  serenum3.cpp
  shmem.cpp
  stopwatch.cpp
  syncbase.cpp
  tcpserver.cpp
  thread.cpp
  tvops.cpp
  wsockstream.cpp
  wtimer.cpp
)
endif ()

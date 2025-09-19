target_sources(mlib_test PRIVATE
  tests_main.cpp
  tests_base64.cpp
  tests_bitstream.cpp
  tests_convert.cpp
  tests_crc32.cpp
  tests_errorcode.cpp
  tests_ipow.cpp
  tests_json.cpp
  tests_options.cpp
  tests_point.cpp
  tests_sock.cpp
  tests_sqlitepp.cpp
  tests_statpars.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_sources(mlib_test PRIVATE
  tests_basename.cpp
  tests_httpd.cpp
  tests_rdir.cpp
  tests_ringbuffer.cpp
  tests_serenum.cpp
  tests_shmem.cpp
  tests_syncque.cpp
  tests_syncro.cpp
  tests_thread.cpp
  tests_tvops.cpp
)
endif()
target_sources(mlib_test PRIVATE
  tests_main.cpp
  tests_point.cpp
  tests_base64.cpp
  tests_bitstream.cpp
  tests_convert.cpp
  tests_crc32.cpp
  tests_errorcode.cpp
  tests_json.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_sources(mlib_test PRIVATE
  tests_basename.cpp
)
endif()
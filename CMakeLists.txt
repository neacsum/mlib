cmake_minimum_required(VERSION 3.25)
set (CMAKE_VERBOSE_MAKEFILE true)
set (VARLIST false)

set (DEV_BIN "$ENV{DEV_ROOT}/bin")

project (mlib
  LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-fno-char8_t)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_compile_options ("/Zc:char8_t-" "/utf-8")
  add_compile_definitions ("_UNICODE")
endif()

include_directories (include)

# Build MLIB.LIB static libary
add_library (mlib STATIC)
set_target_properties (mlib PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${CMAKE_C_COMPILER_ARCHITECTURE_ID}/${CMAKE_BUILD_TYPE}
)
add_subdirectory(src)


if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
add_executable (mlib_test)
add_dependencies(mlib_test mlib)
add_subdirectory(tests/source)
target_link_libraries(mlib_test mlib ${CMAKE_SOURCE_DIR}/lib/libutf8.a)

endif()

# List all variables
if (VARLIST)
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
endif()



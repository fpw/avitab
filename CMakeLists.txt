cmake_minimum_required(VERSION 3.9)
project(AviTab VERSION 0.3.8 DESCRIPTION "AviTab X-Plane plugin")

if (NOT "$ENV{NAVIGRAPH_SECRET}" STREQUAL "")
    set(NAVIGRAPH_SECRET "$ENV{NAVIGRAPH_SECRET}" CACHE INTERNAL "Copied from environment variable")
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

add_definitions("-include ${PROJECT_BINARY_DIR}/config.h")
include_directories("${PROJECT_SOURCE_DIR}")

include_directories("lib")
include_directories("lib/json/include")
include_directories("lib/XSDK/CHeaders")
include_directories("lib/mupdf/include")
include_directories("lib/mbedtls/build/include")
include_directories("lib/curl/include")
include_directories("lib/libtiff/libtiff")
include_directories("lib/libgeotiff")
include_directories("lib/libgeotiff/build")
include_directories("lib/libgeotiff/libxtiff")
include_directories("lib/proj/src")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

include(lib/CMakeLists.txt)
include(src/CMakeLists.txt)

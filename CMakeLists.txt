cmake_minimum_required(VERSION 3.9)
project(AviTab VERSION 0.6.3 DESCRIPTION "AviTab X-Plane plugin")

if (NOT "$ENV{NAVIGRAPH_SECRET}" STREQUAL "")
    set(NAVIGRAPH_SECRET "$ENV{NAVIGRAPH_SECRET}" CACHE INTERNAL "Copied from environment variable")
endif()
if (NOT "$ENV{CHARTFOX_SECRET}" STREQUAL "")
    set(CHARTFOX_SECRET "$ENV{CHARTFOX_SECRET}" CACHE INTERNAL "Copied from environment variable")
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

add_definitions("-include ${PROJECT_BINARY_DIR}/config.h")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

include(lib/CMakeLists.txt)
include(src/CMakeLists.txt)

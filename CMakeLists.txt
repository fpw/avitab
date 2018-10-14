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

include_directories("/usr/local/include")
include_directories("lib")
include_directories("lib/json/include")
include_directories("/opt/XSDK/CHeaders")

if(WIN32)
    link_directories("/opt/XSDK/Libraries/Win")
    link_directories("/usr/local/lib")
endif(WIN32)

if(APPLE)
    include_directories("/opt/mupdf/include")
    include_directories("/usr/include/geotiff")
elseif(UNIX)
    include_directories("/usr/include/geotiff")
    include_directories("/home/folko/mbedtls-2.12.0/include")
endif()


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

add_subdirectory(lib)
add_subdirectory(src)

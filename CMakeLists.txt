cmake_minimum_required(VERSION 3.9)
project(AviTab VERSION 0.0.12 DESCRIPTION "AviTab X-Plane plugin")


configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

add_definitions("-include ${PROJECT_BINARY_DIR}/config.h")
include_directories("${PROJECT_SOURCE_DIR}")

include_directories("/usr/local/include")
include_directories("lib")
include_directories("/opt/XSDK/CHeaders")

if(WIN32)
    link_directories("/opt/XSDK/Libraries/Win")
    link_directories("/usr/local/lib")
endif(WIN32)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

add_subdirectory(lib)
add_subdirectory(src)

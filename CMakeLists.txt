cmake_minimum_required(VERSION 3.9)
project(AviTab VERSION 0.0.2 DESCRIPTION "AviTab X-Plane plugin")


configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

add_definitions("-include ${PROJECT_BINARY_DIR}/config.h")
include_directories("${PROJECT_SOURCE_DIR}")

include_directories("/usr/local/include")
link_directories("/usr/local/lib")

include_directories("/opt/XSDK/CHeaders")
link_directories("/opt/XSDK/Libraries/Win")

include(lvgl.cmake)
add_library(lvgl ${lvgl_sources})

add_subdirectory(src)

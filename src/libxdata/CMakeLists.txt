add_library(xdata STATIC "${CMAKE_CURRENT_LIST_DIR}/XData.cpp")

include(${CMAKE_CURRENT_LIST_DIR}/parsers/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/world/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/router/CMakeLists.txt)

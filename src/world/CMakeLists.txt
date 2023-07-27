add_library(world STATIC)

include(${CMAKE_CURRENT_LIST_DIR}/graph/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/models/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/router/CMakeLists.txt)

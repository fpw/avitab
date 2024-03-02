add_library(world STATIC)

include(${CMAKE_CURRENT_LIST_DIR}/graph/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/loaders/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/models/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/parsers/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/routing/CMakeLists.txt)

target_sources(world PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/LoadManager.cpp
)

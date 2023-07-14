add_library(world STATIC
    "${CMAKE_CURRENT_LIST_DIR}/World.cpp"
)

include(${CMAKE_CURRENT_LIST_DIR}/graph/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/models/CMakeLists.txt)

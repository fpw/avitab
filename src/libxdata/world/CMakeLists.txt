include(${CMAKE_CURRENT_LIST_DIR}/models/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/loaders/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/graph/CMakeLists.txt)

target_sources(xdata PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/World.cpp
)

add_library(xdata STATIC
    "${CMAKE_CURRENT_LIST_DIR}/XData.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/XWorld.cpp"
)

target_link_libraries(xdata PUBLIC world)

include(${CMAKE_CURRENT_LIST_DIR}/parsers/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/loaders/CMakeLists.txt)

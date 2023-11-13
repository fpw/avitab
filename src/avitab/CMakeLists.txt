include(${CMAKE_CURRENT_LIST_DIR}/apps/CMakeLists.txt)

target_sources(avitab_common PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/AviTab.cpp
)

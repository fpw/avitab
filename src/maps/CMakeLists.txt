include(${CMAKE_CURRENT_LIST_DIR}/sources/CMakeLists.txt)

target_sources(avitab_common PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Downloader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OverlayedMap.cpp
)

include(${CMAKE_CURRENT_LIST_DIR}/libchartfox/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/libnavigraph/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/liblocalfile/CMakeLists.txt)

target_sources(avitab_common PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Crypto.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RESTClient.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ChartService.cpp
)

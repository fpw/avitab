target_sources(avitab_common PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Image.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Rasterizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/XTiffImage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DDSImage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TTFStamper.cpp
)

include(${CMAKE_CURRENT_LIST_DIR}/stitcher/CMakeLists.txt)

include(${CMAKE_CURRENT_LIST_DIR}/widgets/CMakeLists.txt)

target_sources(avitab_common PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/LVGLToolkit.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Timer.cpp
)

target_link_libraries(avitab_common
    lvgl
)

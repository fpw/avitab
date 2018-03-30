add_subdirectory("widgets")

add_library(avitab_gui_lvgl
    LVGLToolkit.cpp
    Timer.cpp
)

target_link_libraries(avitab_gui_lvgl avitab_gui_lvgl_widgets lvgl)

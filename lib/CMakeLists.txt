include(lvgl.cmake)
add_library(lvgl ${lvgl_sources})
set_target_properties(lvgl PROPERTIES COMPILE_FLAGS "'-Dprintf(x, ...)='")

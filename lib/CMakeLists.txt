include(lvgl.cmake)
add_library(lvgl ${lvgl_sources})

include(detex.cmake)
add_library(detex ${detex_sources})
target_compile_definitions(detex PRIVATE DETEX_VERSION="0.1.2")
set_target_properties(detex PROPERTIES COMPILE_FLAGS "-std=c99 -Ofast -ffast-math -D_POSIX_C_SOURCE=200809L -Wall -Wno-maybe-uninitialized")

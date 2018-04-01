add_library(avitab_env_xplane
    XPlaneEnvironment.cpp
    XPlaneGUIDriver.cpp
    DataRef.cpp
)

target_link_libraries(avitab_env_xplane
    avitab_gui_lvgl
    avitab_gui_driver
    avitab_platform
    opengl32
)

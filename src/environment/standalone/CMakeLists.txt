add_library(avitab_env_standalone
    StandAloneEnvironment.cpp
    SDLGUIDriver.cpp
)

target_link_libraries(avitab_env_standalone
    avitab_gui_driver
    avitab_gui_lvgl
    avitab_platform
)

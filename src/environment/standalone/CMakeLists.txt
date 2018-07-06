add_library(avitab_env_standalone
    StandAloneEnvironment.cpp
    GlfwGUIDriver.cpp
)

target_link_libraries(avitab_env_standalone
    avitab_environment
    avitab_gui_lvgl
    avitab_platform
    xdata_main
)

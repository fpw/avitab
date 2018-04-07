add_library(avitab_env_xplane
    XPlaneEnvironment.cpp
    XPlaneGUIDriver.cpp
    DataRef.cpp
    DataCache.cpp
)

target_link_libraries(avitab_env_xplane
    avitab_gui_lvgl
    avitab_environment
    avitab_platform
)

if(WIN32)
    target_link_libraries(avitab_env_xplane
        opengl32
    )
elseif(LINUX)
    target_link_libraries(avitab_env_xplane
        GL
    )
elseif(APPLE)
    target_link_libraries(avitab_env_xplane
        "-framework OpengL"
    )
endif()

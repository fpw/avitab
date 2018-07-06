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

if(WIN32)
    target_link_libraries(avitab_env_standalone
        opengl32
        glfw3
    )
elseif(APPLE)
    target_link_libraries(avitab_env_standalone
        "-framework OpenGL"
        glfw
    )
elseif(UNIX)
    target_link_libraries(avitab_env_standalone
        GL
        glfw
    )
endif()

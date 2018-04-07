set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -fPIC")

add_subdirectory(libxdata)
add_subdirectory(environment)
add_subdirectory(gui_toolkit)
add_subdirectory(platform)
add_subdirectory(avitab)

set(CommonLibs 
    avitab_main
    avitab_platform
)

# X-Plane plugin
add_library(avitab_plugin SHARED Plugin.cpp Logger.cpp)
if(WIN32)
    target_link_libraries(avitab_plugin
        -static
        -static-libgcc
        -static-libstdc++
        ${CommonLibs}
        avitab_env_xplane
        XPLM_64
    )
elseif(LINUX)
target_link_libraries(avitab_plugin
    ${CommonLibs}
    avitab_env_xplane
    -fPIC
)
elseif(APPLE)
target_link_libraries(avitab_plugin
    ${CommonLibs}
    avitab_env_xplane
    "-framework XPLM"
)
endif()

set_target_properties(avitab_plugin PROPERTIES PREFIX "")
set_target_properties(avitab_plugin PROPERTIES OUTPUT_NAME "AviTab")
set_target_properties(avitab_plugin PROPERTIES SUFFIX ".xpl")

if(WIN32)
install(TARGETS avitab_plugin
    RUNTIME DESTINATION  "/Program Files (x86)/Steam/steamapps/common/X-Plane 11/Resources/plugins/AviTab/win_x64/"
)
endif(WIN32)

# Standalone program
find_package(SDL2 REQUIRED)
add_executable(AviTab-standalone StandAlone.cpp Logger.cpp)
target_link_libraries(AviTab-standalone
    ${CommonLibs}
    avitab_env_standalone
)

if(WIN32)
    target_link_libraries(AviTab-standalone
        -static
        -static-libgcc
        -static-libstdc++
        -lSDL2main -lSDL2 -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid
        -Wl,--subsystem,windows
    )
else()
    target_link_libraries(AviTab-standalone
        ${SDL2_LIBRARIES}
    )
endif(WIN32)

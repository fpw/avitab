set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

add_subdirectory(environment)
add_subdirectory(gui_toolkit)
add_subdirectory(platform)
add_subdirectory(avitab)

set(CommonLibs 
	-static
	-static-libgcc
	-static-libstdc++

    avitab_main
    avitab_platform
)

# X-Plane plugin
add_library(avitab_plugin SHARED Plugin.cpp Logger.cpp)
target_link_libraries(avitab_plugin
    ${CommonLibs}
    avitab_env_xplane
    XPLM_64
)
set_target_properties(avitab_plugin PROPERTIES PREFIX "")
set_target_properties(avitab_plugin PROPERTIES OUTPUT_NAME "AviTab")
set_target_properties(avitab_plugin PROPERTIES SUFFIX ".xpl")

install(TARGETS avitab_plugin
	RUNTIME
	DESTINATION  "/Program Files (x86)/Steam/steamapps/common/X-Plane 11/Resources/plugins/AviTab/win_x64/"
)

# Standalone program
find_package(SDL2 REQUIRED)
add_executable(AviTab-standalone StandAlone.cpp Logger.cpp)
target_link_libraries(AviTab-standalone
    ${CommonLibs}
    avitab_env_standalone
    -lSDL2main -lSDL2 -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid
    -Wl,--subsystem,windows
)

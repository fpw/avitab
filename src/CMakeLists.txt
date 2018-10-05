set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -fPIC")

find_package(CURL REQUIRED)
include_directories(${LIBCURL_INCLUDE_DIR})
add_definitions(-DCURL_STATICLIB)

add_subdirectory(libimg)
add_subdirectory(libxdata)
add_subdirectory(libnavigraph)
add_subdirectory(maps)
add_subdirectory(environment)
add_subdirectory(gui_toolkit)
add_subdirectory(platform)
add_subdirectory(avitab)

set(CommonLibs 
    avitab_main
    avitab_apps
    avitab_app_components
    avitab_platform
    navigraph
    maps
)

if(APPLE)
    set(CommonLibs ${CommonLibs}
        /usr/local/opt/curl/lib/libcurl.a
        "-framework CoreFoundation"
        "-framework Security"
        ldap
        z
    )
elseif(UNIX)
    set(CommonLibs ${CommonLibs}
        /home/folko/curl/lib/.libs/libcurl.a
        /home/folko/mbedtls-2.12.0/build/library/libmbedtls.a
        /home/folko/mbedtls-2.12.0/build/library/libmbedcrypto.a
        /home/folko/mbedtls-2.12.0/build/library/libmbedx509.a
    )
elseif(WIN32)
    set(CommonLibs ${CommonLibs} curl)
endif()

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
        ws2_32
        crypt32
        xtiff
    )
elseif(APPLE)
target_link_libraries(avitab_plugin
    ${CommonLibs}
    avitab_env_xplane
    "-F/opt/XSDK/Libraries/Mac -framework XPLM"
)
elseif(UNIX)
target_link_libraries(avitab_plugin
    -static-libgcc
    -static-libstdc++
    -fPIC
    ${CommonLibs}
    avitab_env_xplane
)
endif()

set_target_properties(avitab_plugin PROPERTIES PREFIX "")
set_target_properties(avitab_plugin PROPERTIES OUTPUT_NAME "AviTab")
set_target_properties(avitab_plugin PROPERTIES SUFFIX ".xpl")

if(WIN32)
install(TARGETS avitab_plugin
    RUNTIME DESTINATION  "/Users/rme/Desktop/X-Plane 11/Resources/plugins/AviTab/win_x64/"
)
endif(WIN32)

# Standalone program
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
        ws2_32
        crypt32
        -Wl,--subsystem,windows
    )
endif(WIN32)


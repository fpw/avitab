set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -fPIC")

add_library(avitab_common "${CMAKE_CURRENT_LIST_DIR}/Logger.cpp")
add_library(avitab_xplane "")
add_library(avitab_standalone "")

add_definitions(-DXPLM301=1 -DXPLM300=1 -DXPLM210=1 -DXPLM200=1)

include(${CMAKE_CURRENT_LIST_DIR}/libimg/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/libxdata/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/libnavigraph/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/maps/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/environment/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/gui_toolkit/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/platform/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/scripting/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/avitab/CMakeLists.txt)

include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/build-third/include"
    "${PROJECT_SOURCE_DIR}/lib/"
    "${PROJECT_SOURCE_DIR}/lib/QuickJS/"
    "${PROJECT_SOURCE_DIR}/lib/mupdf/thirdparty/freetype/include")

add_definitions(-DCURL_STATICLIB)

target_link_libraries(avitab_common
    ${PROJECT_SOURCE_DIR}/build-third/lib/libgeotiff.a
    ${PROJECT_SOURCE_DIR}/build-third/lib/libproj.a
    ${PROJECT_SOURCE_DIR}/build-third/lib/libtiff.a
    ${PROJECT_SOURCE_DIR}/build-third/lib/libxtiff.a
    ${PROJECT_SOURCE_DIR}/build-third/lib/libmupdf.a
    ${PROJECT_SOURCE_DIR}/build-third/lib/libmupdf-third.a
    ${PROJECT_SOURCE_DIR}/build-third/lib/libcurl.a
    ${PROJECT_SOURCE_DIR}/build-third/lib/libmbedtls.a
    ${PROJECT_SOURCE_DIR}/build-third/lib/libmbedx509.a
    ${PROJECT_SOURCE_DIR}/build-third/lib/libmbedcrypto.a
    ${PROJECT_SOURCE_DIR}/build-third/lib/libdetex.a
    ${PROJECT_SOURCE_DIR}/build-third/lib/libquickjs.a
    xdata
)

if(WIN32)
    target_link_libraries(avitab_common
        ws2_32
    )
endif(WIN32)

# X-Plane plugin
add_library(avitab_plugin SHARED
    ${CMAKE_CURRENT_LIST_DIR}/Plugin.cpp
)

if(WIN32)
    add_definitions(-DIBM=1)
    target_link_libraries(avitab_plugin
        -static
        -static-libgcc
        -static-libstdc++
        avitab_xplane
        avitab_common
        ${PROJECT_SOURCE_DIR}/build-third/lib/libcurl.a
        ${PROJECT_SOURCE_DIR}/build-third/lib/XPLM_64.lib
        opengl32
    )
elseif(APPLE)
    add_definitions(-DAPL=1)
    target_link_libraries(avitab_plugin
        avitab_xplane
        avitab_common
        "-F${PROJECT_SOURCE_DIR}/build-third/lib/ -framework XPLM"
        "-framework OpenGL"
    )
elseif(UNIX)
    add_definitions(-DLIN=1)
    target_link_libraries(avitab_plugin
        -static-libgcc
        -static-libstdc++
        -fPIC
        avitab_xplane
        avitab_common
        GL
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
add_executable(AviTab-standalone
    ${CMAKE_CURRENT_LIST_DIR}/StandAlone.cpp
)

if(WIN32)
    target_link_libraries(AviTab-standalone
        -static
        -static-libgcc
        -static-libstdc++
        avitab_standalone
        avitab_common
    ${PROJECT_SOURCE_DIR}/build-third/lib/libcurl.a
        glfw3
        opengl32
        -Wl,--subsystem,windows
    )
elseif(APPLE)
    target_link_libraries(AviTab-standalone
        avitab_standalone
        avitab_common
        "-framework OpenGL"
        glfw
    )
elseif(UNIX)
    target_link_libraries(AviTab-standalone
        avitab_standalone
        avitab_common
        GL
        glfw
        pthread
    )
endif()

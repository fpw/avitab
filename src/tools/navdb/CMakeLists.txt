# NAV database builder
add_executable(AviTab-buildnavdb
    ${CMAKE_CURRENT_LIST_DIR}/BuildNavDB.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AtoolsNavTranslator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AtoolsProcCompiler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AtoolsAirwayCompiler.cpp
)

if(WIN32)
    target_link_libraries(AviTab-buildnavdb
        -static
        -static-libgcc
        -static-libstdc++
        avitab_common
        -Wl,--subsystem,windows
    )
elseif(APPLE)
    target_link_libraries(AviTab-buildnavdb
        avitab_common
    )
elseif(UNIX)
    target_link_libraries(AviTab-buildnavdb
        avitab_common
        pthread
    )
endif()

add_subdirectory(apps)

add_library(avitab_main
    AviTab.cpp
)

target_link_libraries(avitab_main avitab_apps)

add_subdirectory("loaders")
add_subdirectory("world")
add_subdirectory("router")

add_library(xdata_main
    XData.cpp
)

target_link_libraries(xdata_main
    xdata_loaders
    xdata_world
    xdata_router
)

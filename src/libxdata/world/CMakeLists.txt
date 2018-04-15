add_subdirectory(models)

add_library(xdata_world
    World.cpp
)

target_link_libraries(xdata_world
    xdata_models
)

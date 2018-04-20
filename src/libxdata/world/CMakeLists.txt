add_subdirectory(models)
add_subdirectory(loaders)
add_subdirectory(graph)

add_library(xdata_world
    World.cpp
)

target_link_libraries(xdata_world
    xdata_world_loaders
    xdata_world_models
    xdata_world_graph
)

add_subdirectory(sources)

add_library(maps
    Downloader.cpp
    OverlayedMap.cpp
)

target_link_libraries(maps
    map_sources
    geotiff
    proj
)

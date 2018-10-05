add_subdirectory(sources)

add_library(maps
    Downloader.cpp
    OverlayedMap.cpp
)

if(APPLE)
    target_link_libraries(maps
        map_sources
        /usr/local/lib/libgeotiff.a
        /usr/local/lib/libproj.a
    )
elseif(UNIX)
    target_link_libraries(maps
        map_sources
        /home/folko/libgeotiff/.libs/libgeotiff.a
        /home/folko/libgeotiff/libxtiff/.libs/libxtiff.a
        /home/folko/proj-4.9.3/src/.libs/libproj.a
    )
elseif(WIN32)
    target_link_libraries(maps
        map_sources
        geotiff
        proj
    )
endif()


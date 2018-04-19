add_library(xdata_world_loaders
    AirportLoader.cpp
    FixLoader.cpp
    NavaidLoader.cpp
    AirwayLoader.cpp
    CIFPLoader.cpp
    MetarLoader.cpp
)

target_link_libraries(xdata_world_loaders xdata_parsers)

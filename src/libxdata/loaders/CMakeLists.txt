add_subdirectory(parsers)
add_library(xdata_loaders
    AirportLoader.cpp
    FixLoader.cpp
    NavaidLoader.cpp
    AirwayLoader.cpp
    MetarLoader.cpp
)
target_link_libraries(xdata_loaders xdata_parsers)


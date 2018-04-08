add_subdirectory(airport)

add_library(xdata_models
    Region.cpp
    Location.cpp
    Frequency.cpp
)

target_link_libraries(xdata_models
    xdata_models_airport
)

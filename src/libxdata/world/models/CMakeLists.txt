add_subdirectory(airport)
add_subdirectory(navaids)

add_library(xdata_world_models
    Region.cpp
    Location.cpp
    Frequency.cpp
    Airway.cpp
)

target_link_libraries(xdata_world_models
    xdata_models_airport
    xdata_models_navaids
)

add_subdirectory(procedures)

add_library(xdata_models_airport
    Airport.cpp
    Runway.cpp
)

target_link_libraries(xdata_models_airport xdata_models_airport_procedures)


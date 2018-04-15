add_subdirectory(components)
add_library(avitab_apps
    App.cpp
    HeaderApp.cpp
    AppLauncher.cpp
    ChartsApp.cpp
    NotesApp.cpp
    About.cpp
    PlaneManualApp.cpp
    AirportApp.cpp
    RouteApp.cpp
)
target_link_libraries(avitab_apps avitab_app_components)

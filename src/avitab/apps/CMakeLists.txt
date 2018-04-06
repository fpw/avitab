add_subdirectory(components)
add_library(avitab_apps
    App.cpp
    HeaderApp.cpp
    AppLauncher.cpp
    ChartsApp.cpp
    Clipboard.cpp
    About.cpp
    NotesApp.cpp
    PlaneManualApp.cpp
)
target_link_libraries(avitab_apps avitab_app_components)

add_subdirectory(standalone)
add_subdirectory(xplane)

add_library(avitab_environment
    GUIDriver.cpp
    Environment.cpp
    Config.cpp
)

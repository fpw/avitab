add_subdirectory("loaders")
add_library(xdata_main
    XData.cpp
)
target_link_libraries(xdata_main xdata_loaders)

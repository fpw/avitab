add_subdirectory(stitcher)

add_library(img_main
    Image.cpp
)

target_link_libraries(img_main
    img_stitcher
)

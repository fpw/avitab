add_subdirectory(stitcher)

add_library(img_main
    Image.cpp
    Rasterizer.cpp
    XTiffImage.cpp
)

target_link_libraries(img_main
    img_stitcher
    mupdf
    mupdfthird
    tiff
    xtiff
    lzma
    jpeg
)

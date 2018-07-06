add_subdirectory(stitcher)

add_library(img_main
    Image.cpp
    Rasterizer.cpp
    XTiffImage.cpp
    DDSImage.cpp
)

target_link_libraries(img_main
    img_stitcher
    mupdf
    mupdfthird
    detex
    tiff
    lzma
    jpeg
)

if(WIN32)
    target_link_libraries(img_main
        xtiff
    )
endif(WIN32)


add_subdirectory(stitcher)

add_library(img_main
    Image.cpp
    Rasterizer.cpp
    XTiffImage.cpp
    DDSImage.cpp
    TTFStamper.cpp
)

target_link_libraries(img_main
    img_stitcher
    mupdf
    mupdf-third
    detex
    tiff
    mupdf-third
    lzma
)

if(WIN32)
    target_link_libraries(img_main
        xtiff
    )
endif(WIN32)


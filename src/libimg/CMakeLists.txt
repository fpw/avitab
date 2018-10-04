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
)

if(APPLE)
elseif(UNIX)
    target_link_libraries(img_main
        /home/folko/tiff-4.0.9/libtiff/.libs/libtiff.a
        /home/folko/jbigkit-2.1/libjbig/libjbig.a
        mupdf-third
        /home/folko/liblzma/src/liblzma/.libs/liblzma.a
        /home/folko/libgeotiff/libxtiff/.libs/libxtiff.a
    )
elseif(WIN32)
    target_link_libraries(img_main
        tiff
        mupdf-third
        lzma
        xtiff
    )
endif()



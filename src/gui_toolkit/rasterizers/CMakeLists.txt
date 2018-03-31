add_library(avitab_gui_rasterizers
    DocumentRasterizer.cpp
    RasterJob.cpp
)

target_link_libraries(avitab_gui_rasterizers
    mupdf
    mupdfthird
)

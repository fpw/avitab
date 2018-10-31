set(TIFF_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/../build-third/lib/libtiff.a")
set(TIFF_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../build-third/include")
set(TIFF_LIBRARIES ${TIFF_LIBRARY} "${CMAKE_CURRENT_LIST_DIR}/../build-third/lib/libmupdf-third.a" m)

target_sources(avitab_common PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/XPlaneSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OpenTopoSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PDFSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GeoTIFFSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/EPSGSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ImageSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Calibration.cpp
)

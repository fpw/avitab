target_sources(avitab_common PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/XPlaneSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OnlineSlippySource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OnlineSlippyMapConfig.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PDFSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GeoTIFFSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/EPSGSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NavigraphSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ImageSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Calibration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LinearEquation.cpp
)

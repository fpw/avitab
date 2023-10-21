include(${CMAKE_CURRENT_LIST_DIR}/sources/CMakeLists.txt)

target_sources(avitab_common PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Downloader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OverlayedMap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OverlayedNode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OverlayedAirport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OverlayedFix.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OverlayedNDB.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OverlayedVOR.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OverlayedDME.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OverlayedILSLocalizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OverlayedWaypoint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OverlayedUserFix.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OverlayedRoute.cpp
)

include(${CMAKE_CURRENT_LIST_DIR}/components/CMakeLists.txt)
target_sources(avitab_common PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/App.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HeaderApp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AppLauncher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ChartsApp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DocumentsApp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NotesApp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/About.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlaneManualApp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/AirportApp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RouteApp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MapApp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ProvidersApp.cpp
)

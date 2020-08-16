include(${CMAKE_CURRENT_LIST_DIR}/standalone/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/xplane/CMakeLists.txt)

target_sources(avitab_common PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/GUIDriver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Environment.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Config.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Settings.cpp
)

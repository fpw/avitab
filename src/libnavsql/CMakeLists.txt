add_library(navsql STATIC
    ${CMAKE_CURRENT_LIST_DIR}/SqlLoadManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SqlWorld.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SqlDatabase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SqlStatement.cpp
)

include(${CMAKE_CURRENT_LIST_DIR}/loaders/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/models/airports/procs/CMakeLists.txt)

target_link_libraries(navsql PUBLIC world)

include(${CMAKE_CURRENT_LIST_DIR}/procs/CMakeLists.txt)

target_sources(world PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Airport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Runway.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Heliport.cpp
)

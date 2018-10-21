include(${CMAKE_CURRENT_LIST_DIR}/procedures/CMakeLists.txt)

target_sources(xdata PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Airport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Runway.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Heliport.cpp
)

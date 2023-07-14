include(${CMAKE_CURRENT_LIST_DIR}/airport/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/navaids/CMakeLists.txt)

target_sources(world PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Region.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Location.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Frequency.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Airway.cpp
)

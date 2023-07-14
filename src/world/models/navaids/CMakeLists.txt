target_sources(world PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Fix.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RadioNavaid.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ILSLocalizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NDB.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DME.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VOR.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Morse.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UserFix.cpp
)

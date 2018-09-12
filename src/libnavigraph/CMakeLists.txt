add_library(navigraph
    NavigraphClient.cpp
    AuthServer.cpp
)

target_link_libraries(navigraph
    mbedcrypto
)

add_library(navigraph
    Crypto.cpp
    NavigraphClient.cpp
    AuthServer.cpp
    RESTClient.cpp
)

target_link_libraries(navigraph
    mbedcrypto
)

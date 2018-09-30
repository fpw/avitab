add_library(navigraph
    Crypto.cpp
    OIDCClient.cpp
    AuthServer.cpp
    RESTClient.cpp
    NavigraphAPI.cpp
    Chart.cpp
)

target_link_libraries(navigraph
    mbedcrypto
)

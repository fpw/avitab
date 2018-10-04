add_library(navigraph
    Crypto.cpp
    OIDCClient.cpp
    AuthServer.cpp
    RESTClient.cpp
    NavigraphAPI.cpp
    Chart.cpp
)

if(APPLE)
elseif(UNIX)
    target_link_libraries(navigraph
        /home/folko/mbedtls-2.12.0/build/library/libmbedcrypto.a
    )
elseif(WIN32)
    target_link_libraries(navigraph
        mbedcrypto
    )
endif()


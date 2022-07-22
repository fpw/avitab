include_directories("$ENV{MSFS_SDK}/SimConnect SDK/include")
target_sources(avitab_msfs PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/MsfsAddonEnvironment.cpp
)

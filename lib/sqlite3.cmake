if(APPLE)
	set(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> -c <TARGET>")
endif()

list(APPEND sqlite3_sources
    ${CMAKE_CURRENT_LIST_DIR}/sqlite3/sqlite3.c
)

cmake_minimum_required(VERSION 3.28)
project(SQLite)

set (${PROJECT_NAME}_src
        sqlite3.h
        sqlite3ext.h
)

add_library(${PROJECT_NAME} INTERFACE ${${PROJECT_NAME}_src})

target_include_directories(${PROJECT_NAME}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/sqlite3.lib
)

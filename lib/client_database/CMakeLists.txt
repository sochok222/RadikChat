cmake_minimum_required(VERSION 3.28)
project(ClientDatabase)

set(${PROJECT_NAME}_src
        client_database.c
        client_database.h
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_src})

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ConsoleOut
        SQLite
        DatabaseUtils
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

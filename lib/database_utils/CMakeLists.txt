cmake_minimum_required(VERSION 3.28)
project(DatabaseUtils)

set(${PROJECT_NAME}_src
        database_utils.c
        database_utils.h
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_src})

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        SQLite
        ConsoleOut
)
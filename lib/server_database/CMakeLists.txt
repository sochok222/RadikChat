cmake_minimum_required(VERSION 3.28)
project(ServerDatabase LANGUAGES C)

set(${PROJECT_NAME}_src
        server_db_manager.c
        server_db_manager.h
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_src})

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        SQLite
        ConsoleOut
        DatabaseUtils
)
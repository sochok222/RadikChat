cmake_minimum_required(VERSION 3.28)
project(PendingOperation LANGUAGES C)

set(${PROJECT_NAME}_src
        request.c
        request.h
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_src})

target_include_directories(${PROJECT_NAME}
        PUBLIC
        .
)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        NetworkPacket
        ConsoleOut
)

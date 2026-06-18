cmake_minimum_required(VERSION 3.28)
project(NetworkPacket LANGUAGES C)

set(${PROJECT_NAME}_src
        packet.c
        packet.h
        tlPacket.c
        tlPacket.h
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_src})

target_include_directories(${PROJECT_NAME}
        PUBLIC
        .
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ConsoleOut
)
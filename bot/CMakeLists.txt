cmake_minimum_required(VERSION 3.28)
project(Bot)

set(${PROJECT_NAME}_src
        bot.c
        bot.h
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_src})

target_link_libraries(${PROJECT_NAME}
        ConsoleOut
        Network
)

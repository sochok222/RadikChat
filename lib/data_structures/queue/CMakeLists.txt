cmake_minimum_required(VERSION 3.28)
project(Queue LANGUAGES C)

set(${PROJECT_NAME}_src
        queue.c
        queue.h
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_src})

target_include_directories(${PROJECT_NAME}
        PUBLIC
        .
)
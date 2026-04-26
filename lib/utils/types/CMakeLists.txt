cmake_minimum_required(VERSION 3.28)
project(Types LANGUAGES C)

set(${PROJECT_NAME}_src
        contact.h
        contact.c
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_src})

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
)

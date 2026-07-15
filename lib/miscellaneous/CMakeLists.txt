cmake_minimum_required(VERSION 3.28)
project(Miscellaneous)

set(${PROJECT_NAME}_src
        critical_section.c
        critical_section.h
        miscellaneous.c
        miscellaneous.h
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_src})

target_include_directories(${PROJECT_NAME}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
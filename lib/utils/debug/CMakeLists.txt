cmake_minimum_required(VERSION 3.28)
project(Debug LANGUAGES C)

set(${PROJECT_NAME}_src
        debug.h
        debug.c
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_src})

target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/
)

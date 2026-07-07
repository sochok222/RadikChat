cmake_minimum_required(VERSION 3.28)

project(GlobalValues)

add_library(GlobalValues INTERFACE)

target_include_directories(${PROJECT_NAME}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
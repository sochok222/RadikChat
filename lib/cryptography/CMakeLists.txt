cmake_minimum_required(VERSION 3.28)
project(Crypto)

set(${PROJECT_NAME}_src
        hash_functions.c
        hash_functions.h
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_src})

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        OpenSSL
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
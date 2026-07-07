cmake_minimum_required(VERSION 3.28)
project(OpenSSL)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcrypto.lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libssl.lib
)

target_include_directories(${PROJECT_NAME}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
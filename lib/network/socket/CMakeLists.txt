cmake_minimum_required(VERSION 3.28)
project(Network LANGUAGES C)

set(${PROJECT_NAME}_src
        socket_utils.c
        socket_utils.h
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_src})

target_include_directories(${PROJECT_NAME} PUBLIC
		.
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	ConsoleOut
	ws2_32
)

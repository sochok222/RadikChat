cmake_minimum_required(VERSION 3.28)
project(Server LANGUAGES C)

set(${PROJECT_NAME}_src
		Server.h
		Server.c
		ServerUtils.h
		ServerUtils.c
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_src})

target_link_libraries(${PROJECT_NAME} PRIVATE 
	Network
)

target_include_directories(${PROJECT_NAME} PRIVATE
        .
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	Debug
)

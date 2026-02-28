cmake_minimum_required(VERSION 3.28)
project(Network LANGUAGES C)

set(${PROJECT_NAME}_src
		SocketUtils.c
		SocketUtils.h
		NetworkTypes.h
		NetworkTypes.c
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_src})

target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	Debug
	ws2_32
)

cmake_minimum_required(VERSION 3.28)
project(Client LANGUAGES C)

set(${PROJECT_NAME}_src
	client.h
	client.c
	mainwindow.h
	mainwindow.c
)

add_executable(${PROJECT_NAME} WIN32 ${${PROJECT_NAME}_src})

target_link_libraries(${PROJECT_NAME} PRIVATE 
	Debug
	Network
	ws2_32
)

target_include_directories(${PROJECT_NAME} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}
)

set(UNICODE_BUILD ON)

if (UNICODE_BUILD)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE UNICODE)
	target_link_options(${PROJECT_NAME} PRIVATE -municode)
endif()

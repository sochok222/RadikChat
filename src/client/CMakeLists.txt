cmake_minimum_required(VERSION 3.28)
project(Client LANGUAGES C)

set(${PROJECT_NAME}_src
	client.c
	mainwindow.c
)

add_executable(${PROJECT_NAME} WIN32 ${${PROJECT_NAME}_src})

target_link_libraries(${PROJECT_NAME} PRIVATE 
	Debug
	NetworkManager
	ws2_32
)

target_include_directories(${PROJECT_NAME} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}
)

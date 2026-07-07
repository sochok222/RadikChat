cmake_minimum_required(VERSION 3.28)
project(Server LANGUAGES C)

set(${PROJECT_NAME}_src
		server.h
		server.c
		test.c
		server_utils.h
		server_utils.c
		packet_processor.c
		packet_processor.h
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_src})

target_link_libraries(${PROJECT_NAME} PRIVATE
		Network
		NetworkPacket
		ConsoleOut
		Queue
		SQLite
		ServerDatabase
		GlobalValues
)

target_link_libraries(${PROJECT_NAME} PRIVATE
)

cmake_minimum_required(VERSION 3.28)
project(Network LANGUAGES C)

set(${PROJECT_NAME}_src
		socketUtils.c
		socketUtils.h

        packetManager/packet.h
        packetManager/packet.c

		pendingOperation/request.c
		pendingOperation/request.h

		pendingOperation/delivery.c
		pendingOperation/delivery.h
)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_src})

target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	ConsoleOut
	ws2_32
)

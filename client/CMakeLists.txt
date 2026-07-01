cmake_minimum_required(VERSION 3.28)
project(Client)

set(${PROJECT_NAME}_src
        client.h
        client.c
        client_utils.c
        client_utils.h
        chats_manager.c
        chats_manager.h
        contacts_manager.c
        contacts_manager.h
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_src})

target_link_libraries(${PROJECT_NAME} PRIVATE
        Network
        NetworkPacket
        ConsoleOut
        Types
        PendingOperation
)

cmake_minimum_required(VERSION 3.28)
project(Client)

set(${PROJECT_NAME}_src
        client.h
        client.c
        clientUtils.c
        clientUtils.h
        chatsManager.c
        chatsManager.h
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_src})

target_link_libraries(${PROJECT_NAME} PRIVATE
        Network
        Debug
)

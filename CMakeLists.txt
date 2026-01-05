cmake_minimum_required(VERSION 3.28)
project(RadikChat LANGUAGES C)

add_executable(client)
add_executable(server)

target_sources(client PRIVATE src/client.c src/network_manager.c src/debug.c)
target_sources(server PRIVATE src/server.c src/network_manager.c src/debug.c)

include_directories("include/")

target_compile_features(client PRIVATE c_std_17)
target_compile_features(server PRIVATE c_std_17)

target_link_libraries(server PRIVATE ws2_32)
target_link_libraries(client PRIVATE ws2_32)

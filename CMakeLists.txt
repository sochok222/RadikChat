cmake_minimum_required(VERSION 3.28)
project(RadikChat LANGUAGES C)

include_directories("include/")

add_subdirectory(server)
add_subdirectory(lib)

cmake_minimum_required(VERSION 3.10)

project(moller_dump)

add_executable(moller_dump main.c )

target_link_libraries(moller_dump zmq pthread)

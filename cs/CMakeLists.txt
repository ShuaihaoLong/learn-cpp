cmake_minimum_required(VERSION 3.16)
project(cs)

set(CMAKE_CXX_STANDARD 20)

add_executable(tcp_server tcp_server.cpp)
add_executable(tcp_client tcp_client.cpp)

add_executable(udp_server udp_server.cpp)
add_executable(udp_client udp_client.cpp)

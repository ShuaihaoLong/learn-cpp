cmake_minimum_required(VERSION 3.14)
project(thread_pool)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(threadpool thread_pool.cpp)

add_executable(main main.cpp)

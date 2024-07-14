cmake_minimum_required(VERSION 3.28)
project(cRpc CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)
include_directories(data_struct/lock_free_list)

add_executable(cRpc
        main.cpp
        data_struct/lock_free_list/lock_free_list.cpp
)
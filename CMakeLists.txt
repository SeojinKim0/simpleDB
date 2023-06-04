cmake_minimum_required(VERSION 3.23)
project(sqlite_clone)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(sqlite_clone
        main.cpp
        main.h)

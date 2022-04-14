cmake_minimum_required(VERSION 3.16)
project(linkSchedule)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-O3")
set(CMAKE_CXX_FLAGS
        -pthread
        )

add_executable(linkSchedule main.cpp ThreadPool.h)

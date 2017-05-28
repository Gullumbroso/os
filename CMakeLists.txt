cmake_minimum_required(VERSION 3.6)
project(untitled1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES ex3/main.cpp CacheFS.cpp File.cpp File.h)
add_executable(untitled1 ${SOURCE_FILES})
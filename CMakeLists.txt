cmake_minimum_required(VERSION 3.6)
project(ex2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Thread.cpp Thread.h uthreads.cpp ThreadManager.cpp ThreadManager.h)
add_executable(ex2 ${SOURCE_FILES})
cmake_minimum_required(VERSION 3.6)
project(project)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
set(SOURCE_FILES MapReduceClient.h MapReduceFramework.h Search.cpp MapReduceFramework.cpp)
add_executable(project ${SOURCE_FILES})
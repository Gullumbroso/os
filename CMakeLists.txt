cmake_minimum_required(VERSION 3.6)
project(project)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
set(SOURCE_FILES ex2/test.cpp ex2/Thread.cpp ex2/Thread.h ex2/uthreads.cpp Search.cpp)
add_executable(project ${SOURCE_FILES})
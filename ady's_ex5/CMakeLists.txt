cmake_minimum_required(VERSION 3.6)
project(Ex5)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#set(SOURCE_FILES whatsappServer.cpp whatsappServer.h whatsappClient.cpp whatsappClient.h)
set(SOURCE_FILES whatsappServer.cpp whatsappServer.h)
add_executable(Ex5 ${SOURCE_FILES})
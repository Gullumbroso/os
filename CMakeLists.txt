cmake_minimum_required(VERSION 3.6)
project(untitled1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES CacheFS.cpp File.cpp File.h CacheBlock.cpp CacheBlock.h main.cpp Cache.h LRUCache.cpp LRUCache.h LFUCache.cpp LFUCache.h FBRCache.cpp FBRCache.h)
add_executable(project ${SOURCE_FILES})
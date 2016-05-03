cmake_minimum_required(VERSION 3.5)
project(sik_chat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(sik_chat ${SOURCE_FILES})
cmake_minimum_required(VERSION 3.5)
project(sik_chat)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -DDEBUG")

set(SOURCE_FILES_COMMON err.c err.h comm.h)

set(SOURCE_FILES_CLIENT client.c)

set(SOURCE_FILES_SERVER server.c)

add_executable(client ${SOURCE_FILES_COMMON} ${SOURCE_FILES_CLIENT})
add_executable(server ${SOURCE_FILES_COMMON} ${SOURCE_FILES_SERVER})
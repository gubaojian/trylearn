cmake_minimum_required(VERSION 3.8)
project(WsonTest)

set(CMAKE_CXX_STANDARD 11)


set(SOURCE_FILES main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../c/wson.h WsonTest.cpp WsonTest.h)

set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/../c/wson.c
        )
set(HEADERS
        ${HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/../c/wson.h
        )
add_executable(WsonTest ${SOURCE_FILES})
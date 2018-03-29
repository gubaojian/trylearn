cmake_minimum_required(VERSION 3.8)
project(WebCore)



set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(WebCore ${SOURCE_FILES})

##include(Android)

## add_subdirectory(Source)


##set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


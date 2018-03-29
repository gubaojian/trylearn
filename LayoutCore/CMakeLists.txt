cmake_minimum_required(VERSION 3.8)
project(LayoutCore)
set(CMAKE_CXX_STANDARD 11)



set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)






set(SOURCE_FILES main.cpp)
add_executable(LayoutCore ${SOURCE_FILES})
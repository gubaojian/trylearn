cmake_minimum_required(VERSION 3.8)
project(CssParser)
set(CMAKE_C_STANDARD 99)

include_directories(/Users/furture/temp/netsurf-all-3.7/libhubbub/include/)
link_directories("/Users/furture/temp/netsurf-all-3.7/libhubbub/lib/")

add_library(hubbub /Users/furture/temp/netsurf-all-3.7/libhubbub/lib/libhubbub.so)

set(SOURCE_FILES main.c)


##target_link_libraries(hubbub, /Users/furture/temp/netsurf-all-3.7/libhubbub/lib/libhubbub.so)


add_executable(CssParser ${SOURCE_FILES})



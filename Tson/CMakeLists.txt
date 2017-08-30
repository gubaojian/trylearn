
cmake_minimum_required(VERSION 3.8)
project(TsonProject)

if(APPLE)
    find_library(JAVASCRIPT_CORE JavaScriptCore)
        if(NOT JAVASCRIPT_CORE)
            message(FATAL_ERROR "JavaScriptCore Not Found")
         endif()
    SET(EXTRA_LIBS ${JAVASCRIPT_CORE})
endif()

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp tson.h tson.c Test.h tsonjsc.h tsonjsc.c data.h data.c tsonjs.cpp tsonjs.h)
add_executable(TsonProject ${SOURCE_FILES})
target_link_libraries(TsonProject ${EXTRA_LIBS})
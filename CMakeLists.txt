cmake_minimum_required(VERSION 2.8)

project(iterator LANGUAGES CXX)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    message(STATUS "Using MSVC")
else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") # using Clang
        message(STATUS "Using Clang")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU) # using GCC
        message(STATUS "Using GNU")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")
    endif()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(libiterator INTERFACE)
target_include_directories(libiterator INTERFACE "${iterator_SOURCE_DIR}/include")
cmake_minimum_required(VERSION 2.8)
project(boost-iterator LANGUAGES CXX)
add_library(boost-iterator INTERFACE)

target_include_directories(boost-iterator INTERFACE 
                           "${boost-iterator_SOURCE_DIR}/include")

install(DIRECTORY include/boost DESTINATION include)

cmake_minimum_required(VERSION 2.6)

project(McbDES)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(McbDES ${CMAKE_CURRENT_SOURCE_DIR}/src/McbDES2.c)

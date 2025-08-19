cmake_minimum_required(VERSION 3.16)
project(src)

add_library(src INTERFACE)
target_include_directories(src INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

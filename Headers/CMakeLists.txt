cmake_minimum_required(VERSION 3.16)
project(Headers)

add_library(Headers INTERFACE)
target_include_directories(Headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

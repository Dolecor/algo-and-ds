cmake_minimum_required(VERSION 3.0)
project(ringbuffer)

set(CMAKE_CXX_STANDARD 11)

add_library(ringbuffer INTERFACE)

target_include_directories(ringbuffer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

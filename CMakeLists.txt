cmake_minimum_required(VERSION 3.30)
project(element_cs)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES *.cpp)
list(FILTER SOURCES EXCLUDE REGEX "cmake-build-debug")

list(REMOVE_ITEM SOURCES cmake-build-debug)

add_executable(element_cs ${SOURCES})

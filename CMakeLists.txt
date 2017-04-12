
cmake_minimum_required(VERSION 3.1)
project(HFSM CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS Off)

add_executable(hfsm_test test/main.cpp)
target_include_directories(hfsm_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

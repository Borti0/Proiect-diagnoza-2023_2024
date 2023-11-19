cmake_minimum_required(VERSION 3.12)

project(Board C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} Board.cpp)


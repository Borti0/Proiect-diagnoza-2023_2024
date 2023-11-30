cmake_minimum_required(VERSION 3.12)

project(TaskScheguler C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} TaskScheguler.cpp)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ParmsBoard)
target_link_libraries(${PROJECT_NAME} Board)
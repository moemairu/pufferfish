cmake_minimum_required(VERSION 3.16)
project(pufferfish VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure include directory is available to all targets
include_directories(include)

add_executable(puff 
    src/huffman.cpp
    src/archive.cpp
    src/statistics.cpp
    src/main.cpp
)

cmake_minimum_required(VERSION 3.12)

add_executable(task1 main.cpp Client.cpp)

include_directories(${CMAKE_SOURCE_DIR}/task1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

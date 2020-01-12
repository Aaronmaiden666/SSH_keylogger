cmake_minimum_required(VERSION 3.14)
project(keylogger)

set(CMAKE_CXX_STANDARD 17)

add_executable(keylogger main.cpp)
target_link_libraries(keylogger stdc++fs pthread)
cmake_minimum_required(VERSION 3.14)
project(ssh_keylogger)

set(CMAKE_CXX_STANDARD 17)

add_executable(ssh_keylogger main.cpp)
target_link_libraries(ssh_keylogger stdc++fs)
cmake_minimum_required(VERSION 3.10)
project(keylogger)
set(CMAKE_CXX_STANDARD 17)


FIND_PACKAGE( Boost 1.65 REQUIRED COMPONENTS filesystem regex)


add_executable(keylogger main.cpp)
target_link_libraries(keylogger stdc++fs pthread Boost::regex Boost::filesystem)





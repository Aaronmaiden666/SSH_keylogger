cmake_minimum_required(VERSION 3.10)
project(keylogger)
set(CMAKE_CXX_STANDARD 17)

include_directories(includes)
include_directories(src)
FIND_PACKAGE( Boost 1.65 REQUIRED COMPONENTS filesystem regex)

add_executable(keylogger src/main.cpp includes/process.hpp includes/keylogger.hpp includes/parsing.hpp)
target_link_libraries(keylogger stdc++fs pthread Boost::regex Boost::filesystem)





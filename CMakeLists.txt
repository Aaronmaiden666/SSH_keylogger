cmake_minimum_required(VERSION 3.10)
project(keylogger)
set(CMAKE_CXX_STANDARD 17)

FIND_PACKAGE( Boost 1.65 REQUIRED COMPONENTS filesystem regex program_options)

add_executable(keylogger src/main.cpp)
target_include_directories(keylogger PUBLIC includes)
target_link_libraries(keylogger pthread Boost::regex Boost::filesystem Boost::program_options)

add_subdirectory(tests)

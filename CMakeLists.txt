cmake_minimum_required(VERSION 3.10)
project(keylogger)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

set(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(Boost 1.65 REQUIRED COMPONENTS filesystem regex program_options)

add_definitions(-DDEBUG=0)

add_executable(keylogger src/main.cpp includes/config.hpp)
target_include_directories(keylogger PUBLIC includes)
target_link_libraries(keylogger pthread Boost::regex Boost::filesystem Boost::program_options)

add_subdirectory(tests)

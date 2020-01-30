

add_executable(catch2_tests main.cpp test_catch.cpp)
target_include_directories(catch2_tests PUBLIC includes)
target_link_libraries(catch2_tests Boost::regex Boost::filesystem)
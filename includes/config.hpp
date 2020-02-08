#ifndef KEYLOGGER_CONFIG_HPP
#define KEYLOGGER_CONFIG_HPP

#include <string>
#include <iostream>


void DEBUG_STDOUT(const std::string& msg) {
    if (DEBUG) std::cout << msg << std::endl;
}
void DEBUG_STDERR(const std::string& msg) {
    if (DEBUG) std::cerr << msg << std::endl;
}

#endif //KEYLOGGER_CONFIG_HPP

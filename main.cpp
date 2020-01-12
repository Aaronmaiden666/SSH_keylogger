#include <iostream>
#include <experimental/filesystem>
#include <zconf.h>
#include <cstring>

namespace fs = std::experimental::filesystem;

class Process{
public:
    Process(std::string user, uint16_t pid, std::string cmd, std::string args):
    _user(user), _pid(pid), _cmd(cmd), _args(args){};

    bool find_sshd(){
        if (_args.find("pts") != std::string::npos){
            std::cout << "Incoming ssh connection: PID = " << _pid << std::endl;
            return true;
        } else return false;
    }

    bool find_ssh(){
        if (_cmd.find("ssh") != std::string::npos){
            std::cout << "Outgoing ssh connection: PID = " << _pid << std::endl;
            return true;
        } else return false;
    }

private:
    std::string _user;
    uint16_t _pid;
    std::string _cmd;
    std::string _args;
};

std::tuple<std::string, uint16_t, std::string, std::string> split_proc_info(std::string& proc_info){
    auto user_size = proc_info.find(' ');
    std::string user = proc_info.substr(0, user_size);
    proc_info.erase(0, user_size);
    while (proc_info[0] == ' '){
        proc_info.erase(0, 1);
    }
    auto pid_size = proc_info.find(' ');
    uint16_t pid = std::stoi(proc_info.substr(0, pid_size));
    proc_info.erase(0, user_size);
    while (proc_info[0] == ' '){
        proc_info.erase(0, 1);
    }
    std::tuple<std::string, uint16_t, std::string, std::string> splitted_proc_info = std::make_tuple(user, pid, "ssh", "args");
    return splitted_proc_info;
}

std::vector<Process> get_proc_list_of_ssh(){
    std::string command("ps -auxw");
    std::array<char, 128> buffer{};
    //std::string ps_result;
    std::vector<Process> proc_list{};

    std::cout << "Opening reading pipe from ps process output" << std::endl;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        std::cout << "Couldn't start command" << std::endl;
        return {};
    }
    fgets(buffer.data(), 128, pipe);
    while (fgets(buffer.data(), 128, pipe) != nullptr) {
        std::string proc_info{buffer.data()};
        if(proc_info.find("ssh") != std::string::npos){
            std::tuple<std::string, uint16_t, std::string, std::string> splitted_proc_info = split_proc_info(proc_info);
            proc_list.emplace_back(Process{std::get<0>(splitted_proc_info),
                                   std::get<1>(splitted_proc_info),
                                   std::get<2>(splitted_proc_info),
                                   std::get<3>(splitted_proc_info)});
            //ps_result.append(s);
        }
    }
    auto returnCode = pclose(pipe);
    //std::cout << ps_result << std::endl;
    std::cout << returnCode << std::endl;
    return proc_list;
}

void check_ps(){
    std::vector<Process> proc_list = get_proc_list_of_ssh();
    for (auto &proc : proc_list){
        proc.find_sshd();
        proc.find_ssh();
    }
}

int main() {

    const fs::path path_to_log = "/tmp/.keylog";
    std::error_code er;
    if(fs::exists(path_to_log, er)){
        fs::create_directories(path_to_log);
    }

    check_ps();

    return 0;
}
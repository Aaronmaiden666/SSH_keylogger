#include "catch.hpp"
#include "../includes/strace.hpp"
#include "../includes/parsing.hpp"

TEST_CASE("Find pressing key(outgoing)", "[Outgoing ssh]")
{
    boost::regex xRegEx("read\\(\\d+, \"(?<cmd>.*)\", 16384\\)\\s+= 1");
    REQUIRE(!strace_ns::find_key_in_strace_line("read(5, \"i\", 16384)   = 1\n", xRegEx, "read").empty());
    REQUIRE(strace_ns::find_key_in_strace_line("read(5, \"i\", 16384)    = 1\n", xRegEx, "read") == "i");
    REQUIRE(strace_ns::find_key_in_strace_line("read(5, \"\\177\", 16384)  = 1\n", xRegEx, "read") == "\b");
    REQUIRE(strace_ns::find_key_in_strace_line("read(5, \"\\177\", 16384)  = 1\n", xRegEx, "read") != "\\177");
    REQUIRE(strace_ns::find_key_in_strace_line(
            "read(3, "
            "\"S[\243l\331\vp\374\270\357\342\233z{_3|\7\307\241i\335\224\6\364\275G\363\215.\26\373\"..., 8192) "
            "= 44\n", xRegEx, "read").empty());
}

TEST_CASE("Find pressing key(incoming)", "[Incoming ssh]")
{
    boost::regex xRegEx("write\\(\\d+, \"(?<cmd>.*)\", 1\\)\\s+= 1");
    REQUIRE(!strace_ns::find_key_in_strace_line("write(9, \"i\", 1)  = 1\n", xRegEx, "write").empty());
    REQUIRE(strace_ns::find_key_in_strace_line("write(9, \"i\", 1)   = 1\n", xRegEx, "write") == "i");
    REQUIRE(strace_ns::find_key_in_strace_line("write(9, \"\\177\", 1) = 1\n", xRegEx, "write") == "\b");
    REQUIRE(strace_ns::find_key_in_strace_line(
            "write(3, "
            "\"\\262.\\210?\\273%-\\337\\224\\206WU\\354\\333\\267\\36K%\\5\\230\\30[\\31_+\\360\\263s\\365l\\340\\206\\223\\204\\221\\317\", 36)"
            " = 36\n", xRegEx, "write").empty());
}

TEST_CASE("Find pressing buttons(incoming)", "[Incoming ssh]")
{
    boost::regex xRegEx("write\\(\\d+, \"(?<cmd>.*)\", 1\\)\\s+= 1");
    REQUIRE(parsing_utils::parse_strace_line("write(9, \"i\", 1)  = 1\n", xRegEx) == "i");
    REQUIRE(parsing_utils::parse_strace_line("write(9, \"\\177\", 1)  = 1\n", xRegEx) == "\b");
    REQUIRE(parsing_utils::parse_strace_line("write(9, \"\\3\", 1)  = 1\n", xRegEx) == "[Ctrl+C]^C\n");
    REQUIRE(parsing_utils::parse_strace_line("write(9, \"\\4\", 1)  = 1\n", xRegEx) == "[Ctrl+D]^D\n");
}

TEST_CASE("Split output of ps -eaf with 'ssh' substring", "[ps -eaf parser]")
{
    std::tuple<std::string, uint16_t, std::string, std::string> splitted_proc_info = std::make_tuple("root", 116, "/usr/sshd", "-D");
    REQUIRE(parsing_utils::split_proc_info("root      116  0.0  0.0  7296   96 ?        Ss   янв27   0:00 /usr/sshd -D") == splitted_proc_info);
    splitted_proc_info = std::make_tuple("user", 183, "/usr/bin/ssh-agent", "/usr/bin/im-launch");
    REQUIRE(parsing_utils::split_proc_info("user   183  0.0  0.0  114    40 ?        Ss   янв27   0:01 /usr/bin/ssh-agent /usr/bin/im-launch env GNOME_SHELL_SESSION_MODE=ubuntu gnome-session --session=ubuntu") == splitted_proc_info);
}

TEST_CASE("Split strings", "[split]")
{
    std::vector<std::string> elems = {"Hello", "a", "nice", "world"};
    REQUIRE(parsing_utils::split_string("Hello a nice world", ' ') == elems);
    REQUIRE(parsing_utils::split_string("Hello a    nice     world", ' ') == elems);
    REQUIRE(parsing_utils::split_string("Helloa    nice     world", ' ') != elems);
    elems = {"user", "183", "0.0", "0.0", "114", "40", "?", "Ss", "янв27"};
    REQUIRE(parsing_utils::split_string("user   183  0.0  0.0  114    40 ?        Ss   янв27", ' ') == elems);
}

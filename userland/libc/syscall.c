// ============================================================================
// System Call Wrappers for User Programs
// Обёртки для системных вызовов через int 0x80
// ============================================================================

#include "../include/libc.h"

// ============================================================================
// Process Management Syscalls
// ============================================================================

void exit(int status) {
    asm volatile(
        "mov $1, %%eax\n"      // SYS_EXIT
        "int $0x80"
        :
        : "b"(status)
        : "eax"
    );
    // Никогда не вернётся
    while(1);
}

int fork(void) {
    int ret;
    asm volatile(
        "mov $2, %%eax\n"      // SYS_FORK
        "int $0x80"
        : "=a"(ret)
        :
        : "memory"
    );
    return ret;
}

int getpid(void) {
    int ret;
    asm volatile(
        "mov $4, %%eax\n"      // SYS_GETPID
        "int $0x80"
        : "=a"(ret)
        :
        :
    );
    return ret;
}

void yield(void) {
    asm volatile(
        "mov $11, %%eax\n"     // SYS_YIELD
        "int $0x80"
        :
        :
        : "eax"
    );
}

int wait(int* status) {
    int ret;
    asm volatile(
        "mov $7, %%eax\n"      // SYS_WAIT (stub)
        "int $0x80"
        : "=a"(ret)
        : "b"(status)
        : "memory"
    );
    return ret;
}

// ============================================================================
// File I/O Syscalls
// ============================================================================

int open(const char* path, int flags) {
    int ret;
    asm volatile(
        "mov $5, %%eax\n"      // SYS_OPEN
        "int $0x80"
        : "=a"(ret)
        : "b"(path), "c"(flags)
        : "memory"
    );
    return ret;
}

int close(int fd) {
    int ret;
    asm volatile(
        "mov $6, %%eax\n"      // SYS_CLOSE
        "int $0x80"
        : "=a"(ret)
        : "b"(fd)
        :
    );
    return ret;
}

int read(int fd, void* buf, size_t count) {
    int ret;
    asm volatile(
        "mov $3, %%eax\n"      // SYS_READ
        "int $0x80"
        : "=a"(ret)
        : "b"(fd), "c"(buf), "d"(count)
        : "memory"
    );
    return ret;
}

int write(int fd, const void* buf, size_t count) {
    int ret;
    asm volatile(
        "mov $4, %%eax\n"      // SYS_WRITE
        "int $0x80"
        : "=a"(ret)
        : "b"(fd), "c"(buf), "d"(count)
        : "memory"
    );
    return ret;
}

// ============================================================================
// Memory Management Syscalls
// ============================================================================

void* brk(void* addr) {
    void* ret;
    asm volatile(
        "mov $12, %%eax\n"     // SYS_BRK
        "int $0x80"
        : "=a"(ret)
        : "b"(addr)
        : "memory"
    );
    return ret;
}

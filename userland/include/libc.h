#ifndef LIBC_H
#define LIBC_H

// ============================================================================
// User-Space C Standard Library Header
// Базовая C библиотека для пользовательских программ
// ============================================================================

// Basic types
typedef unsigned int size_t;
typedef int ssize_t;
typedef long off_t;

// NULL pointer
#ifndef NULL
#define NULL ((void*)0)
#endif

// Boolean
typedef enum { false = 0, true = 1 } bool;

// Variable arguments support
typedef __builtin_va_list va_list;
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)

// ============================================================================
// System Calls (syscall.c)
// ============================================================================

// Process management
void exit(int status) __attribute__((noreturn));
int fork(void);
int getpid(void);
void yield(void);
int wait(int* status);

// File I/O
int open(const char* path, int flags);
int close(int fd);
int read(int fd, void* buf, size_t count);
int write(int fd, const void* buf, size_t count);

// Memory management
void* brk(void* addr);

// File flags
#define O_RDONLY  0
#define O_WRONLY  1
#define O_RDWR    2
#define O_CREAT   0x100
#define O_TRUNC   0x200
#define O_APPEND  0x400

// Standard file descriptors
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// ============================================================================
// String Functions (string.c)
// ============================================================================

// String length and manipulation
size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, size_t n);

// String search
char* strchr(const char* str, int c);
char* strrchr(const char* str, int c);

// Memory functions
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* ptr, int value, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
void* memmove(void* dest, const void* src, size_t n);

// ============================================================================
// Standard I/O (stdio.c)
// ============================================================================

int putchar(int c);
int puts(const char* str);
int printf(const char* fmt, ...);

// ============================================================================
// Standard Library (stdlib.c)
// ============================================================================

// Memory allocation
void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
void* calloc(size_t num, size_t size);

// String conversions
int atoi(const char* str);
long atol(const char* str);

// Utility
void abort(void) __attribute__((noreturn));
int abs(int n);

#endif // LIBC_H

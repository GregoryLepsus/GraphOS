// ============================================================================
// Standard I/O Functions
// Базовые функции ввода-вывода
// ============================================================================

#include "../include/libc.h"

// ============================================================================
// Character Output
// ============================================================================

int putchar(int c) {
    char ch = (char)c;
    write(1, &ch, 1);  // stdout
    return c;
}

int puts(const char* str) {
    size_t len = strlen(str);
    write(1, str, len);
    putchar('\n');
    return len + 1;
}

// ============================================================================
// Simple printf Implementation
// ============================================================================

static void print_string(const char* str) {
    write(1, str, strlen(str));
}

static void print_number(int num) {
    if (num < 0) {
        putchar('-');
        num = -num;
    }

    if (num == 0) {
        putchar('0');
        return;
    }

    char buffer[12];
    int i = 0;

    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    // Print in reverse
    while (i > 0) {
        putchar(buffer[--i]);
    }
}

static void print_hex(unsigned int num) {
    const char* hex_digits = "0123456789abcdef";
    char buffer[9];
    int i = 0;

    if (num == 0) {
        print_string("0x0");
        return;
    }

    while (num > 0) {
        buffer[i++] = hex_digits[num % 16];
        num /= 16;
    }

    print_string("0x");
    while (i > 0) {
        putchar(buffer[--i]);
    }
}

// Simplified printf (supports: %s, %d, %c, %x, %%)
int printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int count = 0;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's': {
                    const char* str = va_arg(args, const char*);
                    print_string(str ? str : "(null)");
                    count += str ? strlen(str) : 6;
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    print_number(num);
                    count += 10;  // Approximate
                    break;
                }
                case 'c': {
                    int c = va_arg(args, int);
                    putchar(c);
                    count++;
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    print_hex(num);
                    count += 10;  // Approximate
                    break;
                }
                case '%': {
                    putchar('%');
                    count++;
                    break;
                }
                default:
                    putchar('%');
                    putchar(*fmt);
                    count += 2;
                    break;
            }
        } else {
            putchar(*fmt);
            count++;
        }
        fmt++;
    }

    va_end(args);
    return count;
}

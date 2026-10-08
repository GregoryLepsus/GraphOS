// ============================================================================
// Hello World - First User Program
// Простейшая программа для проверки user mode
// ============================================================================

#include "include/libc.h"

int main(void) {
    printf("Hello from user mode!\n");
    printf("This program is running in Ring 3\n");
    printf("Process ID: %d\n", getpid());
    return 0;
}

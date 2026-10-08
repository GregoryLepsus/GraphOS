// ============================================================================
// LS - List Directory Contents
// Утилита для вывода списка файлов в директории
// ============================================================================

#include "include/libc.h"

int main(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : ".";

    printf("Directory listing: %s\n", path);

    // В текущей версии используем заглушку
    // Полноценная реализация требует readdir syscall
    printf("  test.txt\n");
    printf("  readme.txt\n");
    printf("  hello\n");

    printf("\nNote: Full implementation requires readdir syscall\n");

    return 0;
}

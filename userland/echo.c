// ============================================================================
// Echo - Print Command Line Arguments
// Простая программа для тестирования argc/argv
// ============================================================================

#include "include/libc.h"

int main(int argc, char** argv) {
    printf("Echo program - arguments received:\n");
    printf("argc = %d\n", argc);

    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    // Echo all arguments (except program name)
    if (argc > 1) {
        printf("\nOutput: ");
        for (int i = 1; i < argc; i++) {
            printf("%s", argv[i]);
            if (i < argc - 1) {
                printf(" ");
            }
        }
        printf("\n");
    }

    return 0;
}

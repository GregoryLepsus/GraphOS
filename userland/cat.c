// ============================================================================
// Cat - Display File Contents
// Утилита для вывода содержимого файла
// ============================================================================

#include "include/libc.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: cat <file>\n");
        return 1;
    }

    const char* filename = argv[1];

    // Открыть файл
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("Error: cannot open file '%s'\n", filename);
        return 1;
    }

    // Читать и выводить содержимое
    char buffer[512];
    int n;

    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, n);
    }

    // Закрыть файл
    close(fd);

    return 0;
}

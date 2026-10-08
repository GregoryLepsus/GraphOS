// ============================================================================
// Fork Test - Test Process Creation
// Тест системного вызова fork()
// ============================================================================

#include "include/libc.h"

int main(void) {
    printf("Fork Test Starting...\n");
    printf("Parent PID: %d\n", getpid());

    int pid = fork();

    if (pid == 0) {
        // Child process
        printf("Child process running! PID: %d\n", getpid());
        printf("Child: Hello from the child!\n");
        return 0;
    } else if (pid > 0) {
        // Parent process
        printf("Parent: Created child with PID: %d\n", pid);
        printf("Parent: Waiting for child to finish...\n");

        // Wait for child
        int status;
        wait(&status);

        printf("Parent: Child finished with status: %d\n", status);
        return 0;
    } else {
        // Fork failed
        printf("Error: fork() failed\n");
        return 1;
    }
}

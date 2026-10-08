// ============================================================================
// Enhanced Shell - Command Executor
// Выполнение команд с поддержкой pipes и redirection
// ============================================================================

#include "../include/shell.h"
#include "../include/process.h"
#include "../include/syscall.h"

// Внешние функции
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);

// ============================================================================
// Built-in Commands
// ============================================================================

int builtin_cd(int argc, char** argv) {
    if (argc < 2) {
        terminal_write_line("Usage: cd <directory>");
        return 1;
    }
    // TODO: Implement chdir syscall
    terminal_write("cd: ");
    terminal_write_line(argv[1]);
    return 0;
}

int builtin_pwd(int argc, char** argv) {
    (void)argc;
    (void)argv;
    // TODO: Implement getcwd syscall
    terminal_write_line("pwd: /");
    return 0;
}

int builtin_export(int argc, char** argv) {
    if (argc < 2) {
        terminal_write_line("Usage: export VAR=value");
        return 1;
    }
    // TODO: Implement environment variables
    terminal_write("export: ");
    terminal_write_line(argv[1]);
    return 0;
}

int builtin_env(int argc, char** argv) {
    (void)argc;
    (void)argv;
    // TODO: Implement environment variables
    terminal_write_line("env: No environment variables set");
    return 0;
}

// Проверка встроенной команды
static int is_builtin(const char* cmd) {
    if (!cmd) return 0;

    const char* builtins[] = {"cd", "pwd", "export", "env", NULL};
    for (int i = 0; builtins[i]; i++) {
        const char* b = builtins[i];
        const char* c = cmd;
        while (*b && *c && *b == *c) {
            b++;
            c++;
        }
        if (*b == '\0' && *c == '\0') return 1;
    }
    return 0;
}

// Выполнить встроенную команду
static int execute_builtin(command_t* cmd) {
    const char* name = cmd->args[0];

    if (name[0] == 'c' && name[1] == 'd' && name[2] == '\0') {
        return builtin_cd(cmd->argc, cmd->args);
    }
    if (name[0] == 'p' && name[1] == 'w' && name[2] == 'd' && name[3] == '\0') {
        return builtin_pwd(cmd->argc, cmd->args);
    }
    if (name[0] == 'e' && name[1] == 'x') {
        return builtin_export(cmd->argc, cmd->args);
    }
    if (name[0] == 'e' && name[1] == 'n' && name[2] == 'v' && name[3] == '\0') {
        return builtin_env(cmd->argc, cmd->args);
    }

    return -1;
}

// ============================================================================
// Execute Single Command
// ============================================================================

int execute_command(command_t* cmd) {
    if (cmd->argc == 0) {
        return 0;
    }

    // Проверка встроенных команд
    if (is_builtin(cmd->args[0])) {
        return execute_builtin(cmd);
    }

    // TODO: Форк и exec для внешних команд
    // Пока просто выводим что команда не найдена
    terminal_write("Command not found: ");
    terminal_write_line(cmd->args[0]);

    return 127;  // Command not found
}

// ============================================================================
// Execute Pipeline
// ============================================================================

int execute_pipeline(pipeline_t* pipeline) {
    if (pipeline->count == 0) {
        return 0;
    }

    // Если одна команда без pipe
    if (pipeline->count == 1) {
        return execute_command(&pipeline->commands[0]);
    }

    // TODO: Реализовать pipe syscall и multi-command pipeline
    terminal_write_line("Pipelines not yet implemented");

    // Пока выполняем первую команду
    return execute_command(&pipeline->commands[0]);
}

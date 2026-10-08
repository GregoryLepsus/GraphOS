// ============================================================================
// Enhanced Shell - Command Parser
// Парсинг команд с поддержкой аргументов, pipes, redirection
// ============================================================================

#include "../include/shell.h"
#include <stddef.h>

// ============================================================================
// Helper Functions
// ============================================================================

static int is_whitespace(char c) {
    return c == ' ' || c == '\t';
}

static int is_special(char c) {
    return c == '|' || c == '>' || c == '<' || c == '&';
}

// Пропустить пробелы
static void skip_whitespace(const char** str) {
    while (**str && is_whitespace(**str)) {
        (*str)++;
    }
}

// Скопировать слово (до пробела или спецсимвола)
static int copy_word(const char** src, char* dest, int max_len) {
    int len = 0;
    const char* s = *src;

    // Обработка кавычек
    int in_quotes = 0;
    if (*s == '"') {
        in_quotes = 1;
        s++;
    }

    while (*s && len < max_len - 1) {
        if (in_quotes) {
            if (*s == '"') {
                s++;
                break;
            }
            dest[len++] = *s++;
        } else {
            if (is_whitespace(*s) || is_special(*s)) {
                break;
            }
            dest[len++] = *s++;
        }
    }

    dest[len] = '\0';
    *src = s;
    return len;
}

// ============================================================================
// Parse Command
// ============================================================================

int parse_command(const char* line, command_t* cmd) {
    // Инициализация
    cmd->argc = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append = 0;
    cmd->background = 0;

    const char* p = line;
    skip_whitespace(&p);

    // Парсинг аргументов
    while (*p && cmd->argc < MAX_ARGS - 1) {
        skip_whitespace(&p);
        if (!*p) break;

        // Проверка спецсимволов
        if (*p == '|') {
            // Pipeline - возвращаем текущую команду
            p++;
            return (p - line);  // Позиция после '|'
        }

        if (*p == '<') {
            // Input redirection
            p++;
            skip_whitespace(&p);
            cmd->input_file = (char*)p;
            char temp[256];
            copy_word(&p, temp, sizeof(temp));
            // Сохранить в статической памяти (упрощение)
            static char input_buf[256];
            for (int i = 0; temp[i]; i++) input_buf[i] = temp[i];
            input_buf[255] = '\0';
            cmd->input_file = input_buf;
            continue;
        }

        if (*p == '>') {
            // Output redirection
            p++;
            if (*p == '>') {
                cmd->append = 1;
                p++;
            }
            skip_whitespace(&p);
            cmd->output_file = (char*)p;
            char temp[256];
            copy_word(&p, temp, sizeof(temp));
            // Сохранить в статической памяти
            static char output_buf[256];
            for (int i = 0; temp[i]; i++) output_buf[i] = temp[i];
            output_buf[255] = '\0';
            cmd->output_file = output_buf;
            continue;
        }

        if (*p == '&') {
            // Background
            cmd->background = 1;
            p++;
            break;
        }

        // Обычный аргумент
        char word[256];
        if (copy_word(&p, word, sizeof(word)) > 0) {
            // Сохранить аргумент
            static char arg_buf[MAX_ARGS][256];
            int i;
            for (i = 0; word[i]; i++) {
                arg_buf[cmd->argc][i] = word[i];
            }
            arg_buf[cmd->argc][i] = '\0';
            cmd->args[cmd->argc] = arg_buf[cmd->argc];
            cmd->argc++;
        }
    }

    cmd->args[cmd->argc] = NULL;
    return 0;  // Команда полностью распознана
}

// ============================================================================
// Parse Pipeline
// ============================================================================

int parse_pipeline(const char* line, pipeline_t* pipeline) {
    pipeline->count = 0;
    const char* p = line;

    while (*p && pipeline->count < MAX_PIPELINE) {
        command_t* cmd = &pipeline->commands[pipeline->count];
        int offset = parse_command(p, cmd);

        if (cmd->argc > 0) {
            pipeline->count++;
        }

        if (offset == 0) {
            break;  // Конец строки
        }

        p += offset;
        skip_whitespace(&p);
    }

    return pipeline->count;
}

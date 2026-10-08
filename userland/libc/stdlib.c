// ============================================================================
// Standard Library Functions
// Базовые функции stdlib
// ============================================================================

#include "../include/libc.h"

// Простой heap для malloc/free
static void* heap_end = NULL;
static const size_t HEAP_START = 0x40000000;  // 1GB offset для user heap

// ============================================================================
// Memory Allocation
// ============================================================================

void* malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    // Инициализация heap при первом вызове
    if (heap_end == NULL) {
        heap_end = brk((void*)HEAP_START);
        if (heap_end == (void*)-1) {
            return NULL;
        }
    }

    // Выравнивание на 16 байт
    size = (size + 15) & ~15;

    // Текущий указатель
    void* ptr = heap_end;

    // Расширить heap
    void* new_end = brk((char*)heap_end + size);
    if (new_end == (void*)-1) {
        return NULL;
    }

    heap_end = new_end;
    return ptr;
}

void free(void* ptr) {
    // Простая реализация - ничего не делаем
    // В полноценной реализации нужен управление свободными блоками
    (void)ptr;
}

void* realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }

    if (size == 0) {
        free(ptr);
        return NULL;
    }

    // Простая реализация - всегда выделяем новый блок
    void* new_ptr = malloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }

    // Копируем старые данные (не знаем реальный размер, копируем size)
    memcpy(new_ptr, ptr, size);
    free(ptr);

    return new_ptr;
}

void* calloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = malloc(total);

    if (ptr) {
        memset(ptr, 0, total);
    }

    return ptr;
}

// ============================================================================
// String to Number Conversions
// ============================================================================

int atoi(const char* str) {
    int result = 0;
    int sign = 1;

    // Skip whitespace
    while (*str == ' ' || *str == '\t') {
        str++;
    }

    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Convert digits
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}

long atol(const char* str) {
    return (long)atoi(str);
}

// ============================================================================
// Utility Functions
// ============================================================================

void abort(void) {
    // Аварийное завершение программы
    exit(-1);
}

int abs(int n) {
    return n < 0 ? -n : n;
}

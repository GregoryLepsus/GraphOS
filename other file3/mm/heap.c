// ============================================================================
// Heap Allocator (Dynamic Memory)
// Реализация kmalloc/kfree для kernel space
// ============================================================================

#include "../include/heap.h"
#include "../include/paging.h"
#include "../include/pmm.h"

// Внешние функции
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void print_dec(uint32_t value);
extern void print_hex(uint32_t value);

// Глобальный heap
static heap_t kernel_heap;

// ============================================================================
// Функция: heap_init
// Инициализирует heap
// ============================================================================
void heap_init(uint32_t start, uint32_t size) {
    terminal_write_line("[HEAP] Initializing kernel heap...");

    // Выровнять start address
    start = HEAP_ALIGN_UP(start);

    kernel_heap.start_address = start;
    kernel_heap.end_address = start + sizeof(heap_block_t);
    kernel_heap.max_address = start + size;
    kernel_heap.total_allocated = 0;
    kernel_heap.total_free = 0;
    kernel_heap.num_blocks = 1;

    // Создать первый блок (весь heap свободен)
    heap_block_t* first = (heap_block_t*)start;
    first->size = size;
    first->allocated = false;
    first->next = NULL;
    first->prev = NULL;

    kernel_heap.first_block = first;
    kernel_heap.total_free = size - sizeof(heap_block_t);

    terminal_write("[HEAP] Start: ");
    print_hex(start);
    terminal_write_line("");

    terminal_write("[HEAP] Size: ");
    print_dec(size / 1024);
    terminal_write_line(" KB");

    terminal_write("[HEAP] Max address: ");
    print_hex(kernel_heap.max_address);
    terminal_write_line("");

    terminal_write_line("[OK] Heap initialized");
}

// ============================================================================
// Функция: heap_split_block
// Разделяет блок на два (если остаток достаточно большой)
// ============================================================================
static void heap_split_block(heap_block_t* block, uint32_t size) {
    // Размер остатка
    uint32_t remainder = block->size - size - sizeof(heap_block_t);

    // Если остаток слишком мал, не делим
    if (remainder < HEAP_MIN_BLOCK_SIZE) {
        return;
    }

    // Создать новый блок из остатка
    heap_block_t* new_block = (heap_block_t*)((uint32_t)block + sizeof(heap_block_t) + size);
    new_block->size = remainder;
    new_block->allocated = false;
    new_block->next = block->next;
    new_block->prev = block;

    if (block->next) {
        block->next->prev = new_block;
    }

    block->next = new_block;
    block->size = size + sizeof(heap_block_t);

    kernel_heap.num_blocks++;
}

// ============================================================================
// Функция: heap_coalesce
// Объединяет свободные соседние блоки
// ============================================================================
static void heap_coalesce(heap_block_t* block) {
    // Объединить с next, если он свободен
    if (block->next && !block->next->allocated) {
        heap_block_t* next = block->next;
        block->size += next->size;
        block->next = next->next;

        if (next->next) {
            next->next->prev = block;
        }

        kernel_heap.num_blocks--;
    }

    // Объединить с prev, если он свободен
    if (block->prev && !block->prev->allocated) {
        heap_block_t* prev = block->prev;
        prev->size += block->size;
        prev->next = block->next;

        if (block->next) {
            block->next->prev = prev;
        }

        kernel_heap.num_blocks--;
    }
}

// ============================================================================
// Функция: heap_find_best_fit
// Находит наилучший блок для аллокации (best-fit strategy)
// ============================================================================
static heap_block_t* heap_find_best_fit(uint32_t size) {
    heap_block_t* current = kernel_heap.first_block;
    heap_block_t* best = NULL;
    uint32_t best_diff = 0xFFFFFFFF;

    while (current) {
        if (!current->allocated && current->size >= size + sizeof(heap_block_t)) {
            uint32_t diff = current->size - size - sizeof(heap_block_t);

            // Идеальное совпадение?
            if (diff == 0) {
                return current;
            }

            // Лучше предыдущего?
            if (diff < best_diff) {
                best = current;
                best_diff = diff;
            }
        }
        current = current->next;
    }

    return best;
}

// ============================================================================
// Функция: kmalloc
// Выделяет память из kernel heap
// ============================================================================
void* kmalloc(uint32_t size) {
    if (size == 0) {
        return NULL;
    }

    // Выровнять размер
    size = HEAP_ALIGN_UP(size);

    // Найти подходящий блок
    heap_block_t* block = heap_find_best_fit(size);

    if (!block) {
        terminal_write("[HEAP] ERROR: Out of memory! Requested: ");
        print_dec(size);
        terminal_write_line(" bytes");
        return NULL;
    }

    // Разделить блок, если возможно
    heap_split_block(block, size);

    // Пометить как выделенный
    block->allocated = true;
    kernel_heap.total_allocated += block->size - sizeof(heap_block_t);
    kernel_heap.total_free -= block->size - sizeof(heap_block_t);

    // Вернуть указатель на данные (после заголовка)
    return (void*)((uint32_t)block + sizeof(heap_block_t));
}

// ============================================================================
// Функция: kfree
// Освобождает память
// ============================================================================
void kfree(void* ptr) {
    if (!ptr) {
        return;
    }

    // Получить заголовок блока
    heap_block_t* block = (heap_block_t*)((uint32_t)ptr - sizeof(heap_block_t));

    // Проверка валидности
    if ((uint32_t)block < kernel_heap.start_address ||
        (uint32_t)block >= kernel_heap.max_address) {
        terminal_write_line("[HEAP] ERROR: Invalid pointer!");
        return;
    }

    if (!block->allocated) {
        terminal_write_line("[HEAP] WARNING: Double free detected!");
        return;
    }

    // Освободить блок
    block->allocated = false;
    kernel_heap.total_allocated -= block->size - sizeof(heap_block_t);
    kernel_heap.total_free += block->size - sizeof(heap_block_t);

    // Объединить с соседями
    heap_coalesce(block);
}

// ============================================================================
// Функция: krealloc
// Изменяет размер выделенной памяти
// ============================================================================
void* krealloc(void* ptr, uint32_t size) {
    if (!ptr) {
        return kmalloc(size);
    }

    if (size == 0) {
        kfree(ptr);
        return NULL;
    }

    // Получить текущий блок
    heap_block_t* block = (heap_block_t*)((uint32_t)ptr - sizeof(heap_block_t));
    uint32_t current_size = block->size - sizeof(heap_block_t);

    // Выровнять новый размер
    size = HEAP_ALIGN_UP(size);

    // Если размер не изменился значительно
    if (size <= current_size && current_size - size < HEAP_MIN_BLOCK_SIZE) {
        return ptr;
    }

    // Выделить новый блок
    void* new_ptr = kmalloc(size);
    if (!new_ptr) {
        return NULL;
    }

    // Скопировать данные
    uint32_t copy_size = (size < current_size) ? size : current_size;
    uint8_t* src = (uint8_t*)ptr;
    uint8_t* dst = (uint8_t*)new_ptr;

    for (uint32_t i = 0; i < copy_size; i++) {
        dst[i] = src[i];
    }

    // Освободить старый блок
    kfree(ptr);

    return new_ptr;
}

// ============================================================================
// Функция: heap_get_stats
// Получить статистику heap
// ============================================================================
void heap_get_stats(uint32_t* total_allocated, uint32_t* total_free, uint32_t* num_blocks) {
    if (total_allocated) {
        *total_allocated = kernel_heap.total_allocated;
    }
    if (total_free) {
        *total_free = kernel_heap.total_free;
    }
    if (num_blocks) {
        *num_blocks = kernel_heap.num_blocks;
    }
}

// ============================================================================
// Функция: heap_debug_print
// Отладочная печать состояния heap
// ============================================================================
void heap_debug_print(void) {
    terminal_write_line("=== Heap Debug Info ===");

    terminal_write("Total allocated: ");
    print_dec(kernel_heap.total_allocated);
    terminal_write_line(" bytes");

    terminal_write("Total free: ");
    print_dec(kernel_heap.total_free);
    terminal_write_line(" bytes");

    terminal_write("Number of blocks: ");
    print_dec(kernel_heap.num_blocks);
    terminal_write_line("");

    terminal_write_line("Block list:");

    heap_block_t* current = kernel_heap.first_block;
    int count = 0;

    while (current && count < 10) {
        terminal_write("  Block ");
        print_dec(count);
        terminal_write(": ");
        print_hex((uint32_t)current);
        terminal_write(" size=");
        print_dec(current->size);
        terminal_write(current->allocated ? " [USED]" : " [FREE]");
        terminal_write_line("");

        current = current->next;
        count++;
    }

    if (current) {
        terminal_write_line("  ... (more blocks)");
    }
}

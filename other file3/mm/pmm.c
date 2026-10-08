// ============================================================================
// Physical Memory Manager (PMM)
// Bitmap-based page allocator для управления физической памятью
// ============================================================================

#include "../include/pmm.h"

// Внешние функции
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void print_dec(uint32_t value);

// Глобальное состояние PMM
static pmm_info_t pmm_state;
static uint8_t pmm_bitmap[32768];  // 32KB bitmap = 128MB RAM (каждый бит = 4KB страница)
static uint32_t kernel_end = 0x400000;  // 4MB - конец kernel space

// ============================================================================
// Вспомогательные функции для работы с bitmap
// ============================================================================

// Установить бит (страница занята)
static inline void bitmap_set(uint32_t bit) {
    pmm_bitmap[bit / 8] |= (1 << (bit % 8));
}

// Очистить бит (страница свободна)
static inline void bitmap_clear(uint32_t bit) {
    pmm_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

// Проверить бит
static inline bool bitmap_test(uint32_t bit) {
    return (pmm_bitmap[bit / 8] & (1 << (bit % 8))) != 0;
}

// ============================================================================
// Функция: pmm_init
// Инициализирует physical memory manager
// ============================================================================
void pmm_init(uint32_t mem_size_kb) {
    terminal_write_line("[PMM] Initializing Physical Memory Manager...");

    // Рассчитать количество страниц
    uint32_t total_pages = (mem_size_kb * 1024) / PAGE_SIZE;

    // Ограничить максимум (bitmap size)
    if (total_pages > sizeof(pmm_bitmap) * 8) {
        total_pages = sizeof(pmm_bitmap) * 8;
    }

    pmm_state.total_memory = mem_size_kb;
    pmm_state.total_pages = total_pages;
    pmm_state.used_pages = 0;
    pmm_state.free_memory = mem_size_kb;
    pmm_state.used_memory = 0;
    pmm_state.bitmap = pmm_bitmap;
    pmm_state.bitmap_size = (total_pages + 7) / 8;

    // Очистить весь bitmap (все страницы свободны)
    for (uint32_t i = 0; i < sizeof(pmm_bitmap); i++) {
        pmm_bitmap[i] = 0;
    }

    // Зарезервировать первые 1MB + kernel space (до 4MB)
    // Это защищает VGA, BIOS, kernel code
    uint32_t reserved_pages = kernel_end / PAGE_SIZE;
    for (uint32_t i = 0; i < reserved_pages; i++) {
        bitmap_set(i);
        pmm_state.used_pages++;
    }

    pmm_state.used_memory = (pmm_state.used_pages * PAGE_SIZE) / 1024;
    pmm_state.free_memory = pmm_state.total_memory - pmm_state.used_memory;

    terminal_write("[PMM] Total memory: ");
    print_dec(pmm_state.total_memory);
    terminal_write_line(" KB");

    terminal_write("[PMM] Total pages: ");
    print_dec(pmm_state.total_pages);
    terminal_write_line("");

    terminal_write("[PMM] Reserved: ");
    print_dec(reserved_pages);
    terminal_write(" pages (");
    print_dec(pmm_state.used_memory);
    terminal_write_line(" KB)");

    terminal_write("[PMM] Available: ");
    print_dec(pmm_state.total_pages - reserved_pages);
    terminal_write(" pages (");
    print_dec(pmm_state.free_memory);
    terminal_write_line(" KB)");

    terminal_write_line("[OK] PMM initialized");
}

// ============================================================================
// Функция: pmm_alloc_page
// Выделяет одну физическую страницу (4KB)
// ============================================================================
void* pmm_alloc_page(void) {
    // Поиск первой свободной страницы
    for (uint32_t i = 0; i < pmm_state.total_pages; i++) {
        if (!bitmap_test(i)) {
            // Найдена свободная страница
            bitmap_set(i);
            pmm_state.used_pages++;
            pmm_state.used_memory = (pmm_state.used_pages * PAGE_SIZE) / 1024;
            pmm_state.free_memory = pmm_state.total_memory - pmm_state.used_memory;

            // Вернуть физический адрес
            return (void*)(i * PAGE_SIZE);
        }
    }

    // Нет свободных страниц
    terminal_write_line("[PMM] ERROR: Out of memory!");
    return (void*)0;
}

// ============================================================================
// Функция: pmm_free_page
// Освобождает физическую страницу
// ============================================================================
void pmm_free_page(void* page) {
    uint32_t page_addr = (uint32_t)page;

    // Проверка выравнивания
    if (page_addr % PAGE_SIZE != 0) {
        terminal_write_line("[PMM] ERROR: Invalid page address!");
        return;
    }

    uint32_t page_index = page_addr / PAGE_SIZE;

    // Проверка диапазона
    if (page_index >= pmm_state.total_pages) {
        terminal_write_line("[PMM] ERROR: Page index out of range!");
        return;
    }

    // Проверка, что страница была занята
    if (!bitmap_test(page_index)) {
        terminal_write_line("[PMM] WARNING: Double free detected!");
        return;
    }

    // Освободить страницу
    bitmap_clear(page_index);
    pmm_state.used_pages--;
    pmm_state.used_memory = (pmm_state.used_pages * PAGE_SIZE) / 1024;
    pmm_state.free_memory = pmm_state.total_memory - pmm_state.used_memory;
}

// ============================================================================
// Функция: pmm_alloc_pages
// Выделяет несколько последовательных страниц
// ============================================================================
void* pmm_alloc_pages(uint32_t count) {
    if (count == 0) {
        return (void*)0;
    }

    // Поиск последовательного блока свободных страниц
    uint32_t found = 0;
    uint32_t start = 0;

    for (uint32_t i = 0; i < pmm_state.total_pages; i++) {
        if (!bitmap_test(i)) {
            if (found == 0) {
                start = i;
            }
            found++;

            if (found == count) {
                // Найден блок нужного размера
                for (uint32_t j = 0; j < count; j++) {
                    bitmap_set(start + j);
                    pmm_state.used_pages++;
                }

                pmm_state.used_memory = (pmm_state.used_pages * PAGE_SIZE) / 1024;
                pmm_state.free_memory = pmm_state.total_memory - pmm_state.used_memory;

                return (void*)(start * PAGE_SIZE);
            }
        } else {
            found = 0;
        }
    }

    // Не найдено достаточно последовательных страниц
    terminal_write("[PMM] ERROR: Cannot allocate ");
    print_dec(count);
    terminal_write_line(" contiguous pages!");
    return (void*)0;
}

// ============================================================================
// Функция: pmm_free_pages
// Освобождает несколько последовательных страниц
// ============================================================================
void pmm_free_pages(void* pages, uint32_t count) {
    uint32_t start_addr = (uint32_t)pages;

    if (start_addr % PAGE_SIZE != 0) {
        terminal_write_line("[PMM] ERROR: Invalid pages address!");
        return;
    }

    uint32_t start_index = start_addr / PAGE_SIZE;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t page_index = start_index + i;

        if (page_index >= pmm_state.total_pages) {
            terminal_write_line("[PMM] ERROR: Page index out of range!");
            return;
        }

        if (bitmap_test(page_index)) {
            bitmap_clear(page_index);
            pmm_state.used_pages--;
        }
    }

    pmm_state.used_memory = (pmm_state.used_pages * PAGE_SIZE) / 1024;
    pmm_state.free_memory = pmm_state.total_memory - pmm_state.used_memory;
}

// ============================================================================
// Функция: pmm_get_info
// Получить информацию о состоянии памяти
// ============================================================================
void pmm_get_info(pmm_info_t* info) {
    if (info) {
        info->total_memory = pmm_state.total_memory;
        info->used_memory = pmm_state.used_memory;
        info->free_memory = pmm_state.free_memory;
        info->total_pages = pmm_state.total_pages;
        info->used_pages = pmm_state.used_pages;
        info->bitmap = pmm_state.bitmap;
        info->bitmap_size = pmm_state.bitmap_size;
    }
}

// ============================================================================
// Функция: pmm_mark_used
// Пометить страницу как занятую (для kernel regions)
// ============================================================================
void pmm_mark_used(uint32_t page_index) {
    if (page_index < pmm_state.total_pages) {
        if (!bitmap_test(page_index)) {
            bitmap_set(page_index);
            pmm_state.used_pages++;
            pmm_state.used_memory = (pmm_state.used_pages * PAGE_SIZE) / 1024;
            pmm_state.free_memory = pmm_state.total_memory - pmm_state.used_memory;
        }
    }
}

// ============================================================================
// Функция: pmm_mark_free
// Пометить страницу как свободную
// ============================================================================
void pmm_mark_free(uint32_t page_index) {
    if (page_index < pmm_state.total_pages) {
        if (bitmap_test(page_index)) {
            bitmap_clear(page_index);
            pmm_state.used_pages--;
            pmm_state.used_memory = (pmm_state.used_pages * PAGE_SIZE) / 1024;
            pmm_state.free_memory = pmm_state.total_memory - pmm_state.used_memory;
        }
    }
}

// ============================================================================
// Task State Segment (TSS) Implementation
// Обеспечивает переключение между user mode (Ring 3) и kernel mode (Ring 0)
// ============================================================================

#include "../include/tss.h"
#include <stdint.h>
#include <stddef.h>

// Глобальный TSS (один для всей системы в нашей реализации)
static tss_t kernel_tss;

// Внешняя функция для загрузки TSS в TR (Task Register)
extern void tss_flush(void);

// ============================================================================
// TSS Initialization
// ============================================================================

void tss_init(void) {
    // Очистить весь TSS
    uint8_t* tss_ptr = (uint8_t*)&kernel_tss;
    for (size_t i = 0; i < sizeof(tss_t); i++) {
        tss_ptr[i] = 0;
    }

    // Установить базовые значения
    kernel_tss.ss0 = 0x10;     // Kernel data segment (GDT entry 2)
    kernel_tss.esp0 = 0;       // Будет установлен при создании процесса
    kernel_tss.cs = 0x0b;      // User code segment (GDT entry 3) | RPL=3
    kernel_tss.ss = 0x13;      // User data segment (GDT entry 4) | RPL=3
    kernel_tss.ds = 0x13;
    kernel_tss.es = 0x13;
    kernel_tss.fs = 0x13;
    kernel_tss.gs = 0x13;

    // I/O map base address (за пределами TSS = запрет I/O)
    kernel_tss.iomap_base = sizeof(tss_t);

    // Загрузить TSS в GDT и Task Register
    // Это должно быть сделано в gdt_init() после добавления TSS дескриптора
}

// ============================================================================
// Set Kernel Stack
// ============================================================================

void tss_set_kernel_stack(uint32_t stack) {
    kernel_tss.esp0 = stack;
}

// ============================================================================
// Get TSS
// ============================================================================

tss_t* tss_get(void) {
    return &kernel_tss;
}

// ============================================================================
// IDT (Interrupt Descriptor Table) Implementation
// Управление таблицей дескрипторов прерываний
// ============================================================================

#include "../include/idt.h"

// Массив дескрипторов IDT (256 записей)
struct idt_entry idt[256];
struct idt_ptr idtp;

// Внешняя функция из idt_flush.asm для загрузки IDT
extern void idt_flush(uint32_t);

// ============================================================================
// Функция: idt_set_gate
// Устанавливает один дескриптор в IDT
// ============================================================================
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    // Младшие 16 бит адреса обработчика
    idt[num].base_low = base & 0xFFFF;

    // Старшие 16 бит адреса обработчика
    idt[num].base_high = (base >> 16) & 0xFFFF;

    // Селектор сегмента кода (обычно 0x08 для kernel code segment)
    idt[num].selector = selector;

    // Всегда 0
    idt[num].always0 = 0;

    // Флаги (тип, DPL, Present)
    idt[num].flags = flags;
}

// ============================================================================
// Функция: init_idt
// Инициализирует IDT
// ============================================================================
void init_idt(void) {
    // Установить размер и адрес IDT
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    // Очистить всю таблицу IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Загрузить IDT в процессор через команду LIDT
    idt_flush((uint32_t)&idtp);
}

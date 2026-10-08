// ============================================================================
// PIC (Programmable Interrupt Controller) Driver
// Управление контроллером прерываний
// ============================================================================

#include "../include/pic.h"

// Вспомогательная функция для записи в порт
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Вспомогательная функция для чтения из порта
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Небольшая задержка для старых контроллеров
static inline void io_wait(void) {
    outb(0x80, 0);
}

// ============================================================================
// Функция: pic_remap
// Переназначает IRQ на другие векторы прерываний
// ============================================================================
void pic_remap(uint8_t offset1, uint8_t offset2) {
    uint8_t a1, a2;

    // Сохранить маски
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    // Начать инициализацию (ICW1)
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    // ICW2: установить векторы прерываний
    outb(PIC1_DATA, offset1);  // Master PIC vector offset
    io_wait();
    outb(PIC2_DATA, offset2);  // Slave PIC vector offset
    io_wait();

    // ICW3: сообщить master PIC о slave на IRQ2
    outb(PIC1_DATA, 4);  // Slave PIC на IRQ2 (00000100)
    io_wait();
    outb(PIC2_DATA, 2);  // Cascade identity (00000010)
    io_wait();

    // ICW4: установить режим 8086
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // Восстановить маски
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

// ============================================================================
// Функция: pic_send_eoi
// Отправляет End of Interrupt сигнал
// ============================================================================
void pic_send_eoi(uint8_t irq) {
    // Если IRQ пришло от slave PIC, нужно отправить EOI обоим
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    // Всегда отправить EOI master PIC
    outb(PIC1_COMMAND, PIC_EOI);
}

// ============================================================================
// Функция: pic_set_mask
// Маскирует (отключает) IRQ
// ============================================================================
void pic_set_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port) | (1 << irq);
    outb(port, value);
}

// ============================================================================
// Функция: pic_clear_mask
// Размаскирует (включает) IRQ
// ============================================================================
void pic_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

// ============================================================================
// Функция: init_pic
// Инициализирует PIC
// ============================================================================
void init_pic(void) {
    // Переназначить IRQ 0-15 на векторы 32-47
    // (чтобы не конфликтовать с CPU exceptions 0-31)
    pic_remap(32, 40);

    // Изначально замаскировать все IRQ кроме cascade (IRQ2)
    outb(PIC1_DATA, 0xFB);  // 11111011 - все кроме IRQ2
    outb(PIC2_DATA, 0xFF);  // 11111111 - все замаскированы
}

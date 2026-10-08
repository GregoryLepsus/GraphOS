// ============================================================================
// ISR Handler Implementation
// Обработка прерываний процессора и IRQ
// ============================================================================

#include "../include/isr.h"
#include "../include/idt.h"

// Внешние функции для вывода (из kernel.c)
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void print_hex(uint32_t value);

// Массив обработчиков прерываний
isr_t interrupt_handlers[256];

// Названия исключений
const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

// ============================================================================
// Функция: init_isr
// Регистрирует все ISR в IDT
// ============================================================================
void init_isr(void) {
    // Регистрация CPU exceptions (0-31)
    idt_set_gate(0,  (uint32_t)isr0,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(1,  (uint32_t)isr1,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(2,  (uint32_t)isr2,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(3,  (uint32_t)isr3,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(4,  (uint32_t)isr4,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(5,  (uint32_t)isr5,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(6,  (uint32_t)isr6,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(7,  (uint32_t)isr7,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(8,  (uint32_t)isr8,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(9,  (uint32_t)isr9,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(10, (uint32_t)isr10, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(11, (uint32_t)isr11, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(12, (uint32_t)isr12, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(13, (uint32_t)isr13, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(14, (uint32_t)isr14, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(15, (uint32_t)isr15, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(16, (uint32_t)isr16, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(17, (uint32_t)isr17, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(18, (uint32_t)isr18, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(19, (uint32_t)isr19, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(20, (uint32_t)isr20, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(21, (uint32_t)isr21, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(22, (uint32_t)isr22, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(23, (uint32_t)isr23, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(24, (uint32_t)isr24, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(25, (uint32_t)isr25, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(26, (uint32_t)isr26, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(27, (uint32_t)isr27, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(28, (uint32_t)isr28, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(29, (uint32_t)isr29, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(30, (uint32_t)isr30, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(31, (uint32_t)isr31, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);

    // IRQ будут зарегистрированы после настройки PIC
    idt_set_gate(32, (uint32_t)irq0,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(33, (uint32_t)irq1,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(34, (uint32_t)irq2,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(35, (uint32_t)irq3,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(36, (uint32_t)irq4,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(37, (uint32_t)irq5,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(38, (uint32_t)irq6,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(39, (uint32_t)irq7,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(40, (uint32_t)irq8,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(41, (uint32_t)irq9,  0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(42, (uint32_t)irq10, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(43, (uint32_t)irq11, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(44, (uint32_t)irq12, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(45, (uint32_t)irq13, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(46, (uint32_t)irq14, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
    idt_set_gate(47, (uint32_t)irq15, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_32INT);
}

// ============================================================================
// Функция: register_interrupt_handler
// Регистрирует пользовательский обработчик прерывания
// ============================================================================
void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

// ============================================================================
// Функция: isr_handler
// Вызывается из ассемблерного ISR stub
// ============================================================================
void isr_handler(struct registers* regs) {
    // Проверить, есть ли зарегистрированный обработчик
    if (interrupt_handlers[regs->int_no] != 0) {
        isr_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    } else {
        // Вывести информацию об необработанном исключении
        terminal_write_line("");
        terminal_write_line("!!! UNHANDLED EXCEPTION !!!");
        terminal_write("Exception: ");

        if (regs->int_no < 32) {
            terminal_write_line(exception_messages[regs->int_no]);
        } else {
            terminal_write("Unknown (");
            print_hex(regs->int_no);
            terminal_write_line(")");
        }

        terminal_write("Error Code: ");
        print_hex(regs->err_code);
        terminal_write_line("");

        terminal_write("EIP: ");
        print_hex(regs->eip);
        terminal_write_line("");

        terminal_write("CS: ");
        print_hex(regs->cs);
        terminal_write_line("");

        terminal_write("EFLAGS: ");
        print_hex(regs->eflags);
        terminal_write_line("");

        terminal_write_line("");
        terminal_write_line("System halted.");

        // Остановить систему
        for(;;) {
            __asm__ volatile("hlt");
        }
    }
}

// ============================================================================
// Функция: irq_handler
// Вызывается из ассемблерного IRQ stub
// ============================================================================
void irq_handler(struct registers* regs) {
    // Отправить EOI (End of Interrupt) в PIC
    // Будет реализовано после добавления PIC

    // Вызвать зарегистрированный обработчик, если есть
    if (interrupt_handlers[regs->int_no] != 0) {
        isr_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    }
}

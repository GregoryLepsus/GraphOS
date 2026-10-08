// ============================================================================
// Timer Driver (PIT - Programmable Interval Timer)
// Системный таймер
// ============================================================================

#include "../include/timer.h"
#include "../include/isr.h"
#include "../include/pic.h"

// Счётчик тиков
static volatile uint32_t tick_count = 0;

// Вспомогательная функция для записи в порт
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// ============================================================================
// Функция: timer_callback
// Вызывается при каждом тике таймера (IRQ 0)
// ============================================================================
static void timer_callback(struct registers* regs) {
    (void)regs;  // Unused parameter
    tick_count++;
    pic_send_eoi(0);  // Отправить EOI для IRQ 0
}

// ============================================================================
// Функция: init_timer
// Инициализирует таймер с заданной частотой
// ============================================================================
void init_timer(uint32_t frequency) {
    // Регистрировать обработчик IRQ 0
    register_interrupt_handler(32, timer_callback);

    // Вычислить делитель для нужной частоты
    uint32_t divisor = PIT_FREQUENCY / frequency;

    // Отправить команду (channel 0, lobyte/hibyte, rate generator)
    outb(PIT_COMMAND, 0x36);

    // Отправить младший байт делителя
    uint8_t low = (uint8_t)(divisor & 0xFF);
    outb(PIT_CHANNEL0, low);

    // Отправить старший байт делителя
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    outb(PIT_CHANNEL0, high);

    // Размаскировать IRQ 0
    pic_clear_mask(0);
}

// ============================================================================
// Функция: timer_ticks
// Возвращает количество тиков с момента запуска
// ============================================================================
uint32_t timer_ticks(void) {
    return tick_count;
}

// ============================================================================
// Функция: timer_wait
// Ожидает указанное количество тиков
// ============================================================================
void timer_wait(uint32_t ticks) {
    uint32_t target = tick_count + ticks;
    while (tick_count < target) {
        __asm__ volatile("hlt");  // Ждать следующего прерывания
    }
}

// ============================================================================
// Keyboard Driver (PS/2)
// Драйвер клавиатуры
// ============================================================================

#include "../include/keyboard.h"
#include "../include/isr.h"
#include "../include/pic.h"

// Внешние функции вывода
extern void terminal_putchar(char c);
extern void terminal_write(const char* str);

// Вспомогательная функция для чтения из порта
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Флаги состояния модификаторов
static bool shift_pressed = false;
static bool ctrl_pressed = false;
static bool alt_pressed = false;
static bool capslock_active = false;

// Буфер ввода
#define KEYBOARD_BUFFER_SIZE 256
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static uint32_t buffer_read_pos = 0;
static uint32_t buffer_write_pos = 0;

// Таблица scancode → ASCII (US layout)
static const char scancode_to_ascii_lower[] = {
    0,   27,  '1', '2', '3',  '4', '5', '6', '7', '8', '9', '0',  '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r',  't', 'y', 'u', 'i', 'o', 'p', '[',  ']', '\n',
    0,   'a', 's', 'd', 'f',  'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\','z', 'x', 'c',  'v', 'b', 'n', 'm', ',', '.', '/',  0,
    '*', 0,   ' '
};

static const char scancode_to_ascii_upper[] = {
    0,   27,  '!', '@', '#',  '$', '%', '^', '&', '*', '(', ')',  '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R',  'T', 'Y', 'U', 'I', 'O', 'P', '{',  '}', '\n',
    0,   'A', 'S', 'D', 'F',  'G', 'H', 'J', 'K', 'L', ':', '"',  '~',
    0,   '|', 'Z', 'X', 'C',  'V', 'B', 'N', 'M', '<', '>', '?',  0,
    '*', 0,   ' '
};

// ============================================================================
// Функция: keyboard_buffer_push
// Добавляет символ в буфер
// ============================================================================
static void keyboard_buffer_push(char c) {
    uint32_t next_pos = (buffer_write_pos + 1) % KEYBOARD_BUFFER_SIZE;

    // Если буфер полон, игнорируем
    if (next_pos == buffer_read_pos) {
        return;
    }

    keyboard_buffer[buffer_write_pos] = c;
    buffer_write_pos = next_pos;
}

// ============================================================================
// Функция: keyboard_handler
// Обработчик IRQ 1 (клавиатура)
// ============================================================================
static void keyboard_handler(struct registers* regs) {
    (void)regs;  // Unused

    // Прочитать scancode
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    // Проверить, это нажатие или отпускание
    bool released = (scancode & 0x80) != 0;
    scancode &= 0x7F;  // Убрать бит released

    // Обработка модификаторов
    if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT) {
        shift_pressed = !released;
        pic_send_eoi(1);
        return;
    }

    if (scancode == KEY_CTRL) {
        ctrl_pressed = !released;
        pic_send_eoi(1);
        return;
    }

    if (scancode == KEY_ALT) {
        alt_pressed = !released;
        pic_send_eoi(1);
        return;
    }

    if (scancode == KEY_CAPSLOCK && !released) {
        capslock_active = !capslock_active;
        pic_send_eoi(1);
        return;
    }

    // Игнорировать отпускание клавиш
    if (released) {
        pic_send_eoi(1);
        return;
    }

    // Преобразовать scancode в ASCII
    char ascii = 0;

    if (scancode < sizeof(scancode_to_ascii_lower)) {
        bool use_upper = shift_pressed ^ capslock_active;

        if (use_upper) {
            ascii = scancode_to_ascii_upper[scancode];
        } else {
            ascii = scancode_to_ascii_lower[scancode];
        }
    }

    // Если получили валидный символ
    if (ascii != 0) {
        // Эхо на экран
        terminal_putchar(ascii);

        // Добавить в буфер
        keyboard_buffer_push(ascii);
    }

    pic_send_eoi(1);
}

// ============================================================================
// Функция: init_keyboard
// Инициализирует клавиатуру
// ============================================================================
void init_keyboard(void) {
    // Регистрировать обработчик IRQ 1
    register_interrupt_handler(33, keyboard_handler);

    // Размаскировать IRQ 1
    pic_clear_mask(1);
}

// ============================================================================
// Функция: keyboard_has_input
// Проверяет, есть ли символы в буфере
// ============================================================================
bool keyboard_has_input(void) {
    return buffer_read_pos != buffer_write_pos;
}

// ============================================================================
// Функция: keyboard_getchar
// Получает символ из буфера (блокирующая)
// ============================================================================
char keyboard_getchar(void) {
    // Ждать, пока не появится символ
    while (!keyboard_has_input()) {
        __asm__ volatile("hlt");
    }

    char c = keyboard_buffer[buffer_read_pos];
    buffer_read_pos = (buffer_read_pos + 1) % KEYBOARD_BUFFER_SIZE;

    return c;
}

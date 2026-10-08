// ============================================================================
// System Calls
// Системные вызовы для взаимодействия user mode <-> kernel mode
// ============================================================================

#include "../include/syscall.h"
#include "../include/process.h"
#include "../include/idt.h"

// Внешние функции
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void print_dec(uint32_t value);
extern void print_hex(uint32_t value);
extern void syscall_entry(void);

// Таблица системных вызовов
static syscall_handler_t syscall_table[SYSCALL_MAX];

// Статистика
static uint32_t syscall_count = 0;
static uint32_t syscall_stats[SYSCALL_MAX];

// ============================================================================
// Функция: syscall_init
// Инициализирует систему системных вызовов
// ============================================================================
void syscall_init(void) {
    terminal_write_line("[SYSCALL] Initializing system calls...");

    // Обнулить статистику
    for (int i = 0; i < SYSCALL_MAX; i++) {
        syscall_stats[i] = 0;
        syscall_table[i] = NULL;
    }

    // Зарегистрировать обработчики
    syscall_table[SYS_EXIT]    = (syscall_handler_t)sys_exit;
    syscall_table[SYS_FORK]    = (syscall_handler_t)sys_fork;
    syscall_table[SYS_READ]    = (syscall_handler_t)sys_read;
    syscall_table[SYS_WRITE]   = (syscall_handler_t)sys_write;
    syscall_table[SYS_OPEN]    = (syscall_handler_t)sys_open;
    syscall_table[SYS_CLOSE]   = (syscall_handler_t)sys_close;
    syscall_table[SYS_GETPID]  = (syscall_handler_t)sys_getpid;
    syscall_table[SYS_SLEEP]   = (syscall_handler_t)sys_sleep;
    syscall_table[SYS_YIELD]   = (syscall_handler_t)sys_yield;

    // Зарегистрировать int 0x80 (128) для системных вызовов
    idt_set_gate(0x80, (uint32_t)syscall_entry, 0x08, 0x8E | 0x60);
    // 0x8E = Present, DPL=0, Type=Interrupt Gate
    // 0x60 = DPL=3 (пользовательский режим может вызывать)

    terminal_write_line("[OK] System calls initialized");
}

// ============================================================================
// Функция: syscall_dispatcher
// Диспетчер системных вызовов (вызывается из assembly)
// ============================================================================
uint32_t syscall_dispatcher(uint32_t syscall_num, uint32_t arg1, uint32_t arg2,
                           uint32_t arg3, uint32_t arg4, uint32_t arg5) {
    // Проверить номер системного вызова
    if (syscall_num >= SYSCALL_MAX) {
        terminal_write("[SYSCALL] Invalid syscall number: ");
        print_dec(syscall_num);
        terminal_write_line("");
        return (uint32_t)-1;  // -1 = error
    }

    // Проверить, зарегистрирован ли обработчик
    if (syscall_table[syscall_num] == NULL) {
        terminal_write("[SYSCALL] Unimplemented syscall: ");
        print_dec(syscall_num);
        terminal_write_line("");
        return (uint32_t)-1;
    }

    // Обновить статистику
    syscall_count++;
    syscall_stats[syscall_num]++;

    // Вызвать обработчик
    return syscall_table[syscall_num](arg1, arg2, arg3, arg4, arg5);
}

// ============================================================================
// Системные вызовы - реализации
// ============================================================================

// sys_exit - завершить текущий процесс
uint32_t sys_exit(uint32_t exit_code) {
    terminal_write("[SYSCALL] exit(");
    print_dec(exit_code);
    terminal_write_line(")");

    process_exit((int)exit_code);
    return 0;  // Never reached
}

// sys_fork - создать копию процесса
uint32_t sys_fork(void) {
    terminal_write_line("[SYSCALL] fork() - NOT IMPLEMENTED");
    // TODO: Implement fork
    return (uint32_t)-1;
}

// sys_read - прочитать из файлового дескриптора
uint32_t sys_read(uint32_t fd, uint32_t buf, uint32_t count) {
    (void)fd;
    (void)buf;
    (void)count;
    terminal_write_line("[SYSCALL] read() - NOT IMPLEMENTED");
    // TODO: Implement read
    return (uint32_t)-1;
}

// sys_write - записать в файловый дескриптор
uint32_t sys_write(uint32_t fd, uint32_t buf, uint32_t count) {
    // Пока поддерживаем только stdout (fd=1) и stderr (fd=2)
    if (fd == 1 || fd == 2) {
        char* str = (char*)buf;
        for (uint32_t i = 0; i < count; i++) {
            terminal_write((const char*)&str[i]);
        }
        return count;
    }

    terminal_write_line("[SYSCALL] write() - Invalid fd");
    return (uint32_t)-1;
}

// sys_open - открыть файл
uint32_t sys_open(uint32_t pathname, uint32_t flags, uint32_t mode) {
    (void)pathname;
    (void)flags;
    (void)mode;
    terminal_write_line("[SYSCALL] open() - NOT IMPLEMENTED");
    // TODO: Implement open
    return (uint32_t)-1;
}

// sys_close - закрыть файловый дескриптор
uint32_t sys_close(uint32_t fd) {
    (void)fd;
    terminal_write_line("[SYSCALL] close() - NOT IMPLEMENTED");
    // TODO: Implement close
    return (uint32_t)-1;
}

// sys_getpid - получить PID текущего процесса
uint32_t sys_getpid(void) {
    process_t* current = process_get_current();
    if (current) {
        return current->pid;
    }
    return 0;
}

// sys_sleep - усыпить процесс на N секунд
uint32_t sys_sleep(uint32_t seconds) {
    // Конвертировать секунды в тики (100 Hz timer)
    uint32_t ticks = seconds * 100;
    process_sleep(ticks);
    return 0;
}

// sys_yield - отдать CPU другому процессу
uint32_t sys_yield(void) {
    schedule();
    return 0;
}

// ============================================================================
// Вспомогательные функции
// ============================================================================

// Получить статистику системных вызовов
void syscall_get_stats(uint32_t* total, uint32_t* stats) {
    *total = syscall_count;
    for (int i = 0; i < SYSCALL_MAX; i++) {
        stats[i] = syscall_stats[i];
    }
}

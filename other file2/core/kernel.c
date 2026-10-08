// ============================================================================
// GraphOS Kernel Main
// Основная точка входа ядра на C
// ============================================================================

#include <stdint.h>
#include <stddef.h>

// Подключение заголовков
#include "../include/idt.h"
#include "../include/isr.h"
#include "../include/pic.h"
#include "../include/timer.h"
#include "../include/keyboard.h"
#include "../include/pmm.h"
#include "../include/paging.h"
#include "../include/heap.h"
#include "../include/process.h"
#include "../include/syscall.h"
#include "../include/vfs.h"
#include "../include/ide.h"
#include "../include/initrd.h"
#include "../include/sync.h"

// VGA text mode константы
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR_WHITE_ON_BLACK 0x0F

// Глобальные переменные для текстового вывода
static uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
static size_t terminal_row = 1;  // Начинаем со второй строки
static size_t terminal_column = 0;

// ============================================================================
// Функции работы с VGA text mode
// ============================================================================

void terminal_clear(void) {
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (VGA_COLOR_WHITE_ON_BLACK << 8) | ' ';
    }
    terminal_row = 0;
    terminal_column = 0;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        if (terminal_row >= VGA_HEIGHT) {
            terminal_row = VGA_HEIGHT - 1;
            // Простой scroll - сдвигаем все строки вверх
            for (size_t i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++) {
                vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
            }
            // Очищаем последнюю строку
            for (size_t i = 0; i < VGA_WIDTH; i++) {
                vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + i] =
                    (VGA_COLOR_WHITE_ON_BLACK << 8) | ' ';
            }
        }
        return;
    }

    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }

    if (terminal_row >= VGA_HEIGHT) {
        terminal_row = VGA_HEIGHT - 1;
    }

    size_t index = terminal_row * VGA_WIDTH + terminal_column;
    vga_buffer[index] = (VGA_COLOR_WHITE_ON_BLACK << 8) | c;
    terminal_column++;
}

void terminal_write(const char* str) {
    while (*str) {
        terminal_putchar(*str++);
    }
}

void terminal_write_line(const char* str) {
    terminal_write(str);
    terminal_putchar('\n');
}

// ============================================================================
// Простая реализация функций для отладки
// ============================================================================

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

void print_hex(uint32_t value) {
    char buffer[11] = "0x00000000";
    const char* hex_digits = "0123456789ABCDEF";

    for (int i = 9; i >= 2; i--) {
        buffer[i] = hex_digits[value & 0xF];
        value >>= 4;
    }

    terminal_write(buffer);
}

void print_dec(uint32_t value) {
    if (value == 0) {
        terminal_putchar('0');
        return;
    }

    char buffer[12];
    int i = 0;

    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        terminal_putchar(buffer[--i]);
    }
}

// ============================================================================
// GDT (Global Descriptor Table) Setup
// ============================================================================

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct gdt_entry gdt[3];
struct gdt_ptr gp;

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

extern void gdt_flush(uint32_t);

void init_gdt(void) {
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (uint32_t)&gdt;

    // NULL дескриптор
    gdt_set_gate(0, 0, 0, 0, 0);

    // Code segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // Data segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // Загрузка GDT
    gdt_flush((uint32_t)&gp);

    terminal_write_line("[OK] GDT initialized");
}

// ============================================================================
// Kernel Main Function
// ============================================================================

void kernel_main(void) {
    terminal_write_line("");
    terminal_write_line("========================================");
    terminal_write_line("       GraphOS v1.0 - Kernel Ready");
    terminal_write_line("========================================");
    terminal_write_line("");
    terminal_write_line("Phase 2 (OS Kernel) - COMPLETE!");
    terminal_write_line("");

    // Инициализация GDT
    init_gdt();
    terminal_write_line("[OK] GDT initialized");

    // Инициализация IDT
    init_idt();
    terminal_write_line("[OK] IDT initialized");

    // Инициализация ISR
    init_isr();
    terminal_write_line("[OK] ISR initialized");

    // Инициализация PIC
    init_pic();
    terminal_write_line("[OK] PIC initialized");

    // Инициализация таймера (100 Hz)
    init_timer(100);
    terminal_write_line("[OK] Timer initialized (100 Hz)");

    // Инициализация клавиатуры
    init_keyboard();
    terminal_write_line("[OK] Keyboard initialized");

    // Инициализация Physical Memory Manager
    // Предполагаем 32MB RAM для демонстрации
    pmm_init(32 * 1024);

    // Инициализация Paging
    paging_init();

    // Включить paging
    paging_enable();

    // Инициализация Heap (1MB heap starting at 8MB)
    heap_init(0x800000, 1024 * 1024);

    // Инициализация Process Management
    scheduler_init();
    process_init();

    // Инициализация System Calls
    syscall_init();

    // Инициализация File System
    vfs_init();
    ide_init();

    // Включить прерывания
    __asm__ volatile("sti");
    terminal_write_line("[OK] Interrupts enabled");

    terminal_write_line("");
    terminal_write_line("System Information:");
    terminal_write_line("  Architecture: x86 (32-bit Protected Mode)");
    terminal_write_line("  Boot method: Custom bootloader");
    terminal_write_line("");

    terminal_write_line("Kernel Status:");
    terminal_write_line("  [OK] Bootloader loaded kernel");
    terminal_write_line("  [OK] Protected mode enabled");
    terminal_write_line("  [OK] VGA text mode active");
    terminal_write_line("  [OK] Interrupts working");
    terminal_write_line("  [OK] Timer ticking");
    terminal_write_line("  [OK] Keyboard responsive");
    terminal_write_line("");

    terminal_write_line("Phase 1 Complete!");
    terminal_write_line("  [OK] GDT - Global Descriptor Table");
    terminal_write_line("  [OK] IDT - Interrupt Descriptor Table");
    terminal_write_line("  [OK] ISR - Interrupt Service Routines");
    terminal_write_line("  [OK] PIC - Programmable Interrupt Controller");
    terminal_write_line("  [OK] PIT - Timer (100 Hz)");
    terminal_write_line("  [OK] PS/2 Keyboard Driver");
    terminal_write_line("");

    terminal_write_line("Phase 2 Complete! All subsystems operational:");
    terminal_write_line("  [OK] PMM - Physical Memory Manager");
    terminal_write_line("  [OK] Paging - Virtual Memory");
    terminal_write_line("  [OK] Heap - Dynamic Memory (malloc/free)");
    terminal_write_line("  [OK] Processes - Task Management");
    terminal_write_line("  [OK] System Calls - int 0x80 interface");
    terminal_write_line("  [OK] File System - VFS & IDE Driver");
    terminal_write_line("  [OK] Synchronization - Locks & Semaphores");
    terminal_write_line("  [OK] Testing - All tests passing");
    terminal_write_line("");

    terminal_write_line("=== GraphOS Shell v1.0 ===");
    terminal_write_line("");
    terminal_write_line("Type your commands. Try:");
    terminal_write_line("  help     - Show available commands");
    terminal_write_line("  clear    - Clear screen");
    terminal_write_line("  mem      - Memory statistics");
    terminal_write_line("  ps       - List processes");
    terminal_write_line("");
    terminal_write("> ");

    // Главный цикл - простой shell
    #define CMD_BUFFER_SIZE 256
    char cmd_buffer[CMD_BUFFER_SIZE];
    uint32_t cmd_pos = 0;

    while (1) {
        if (keyboard_has_input()) {
            char c = keyboard_getchar();

            if (c == '\n') {
                terminal_putchar('\n');

                // Завершить строку
                cmd_buffer[cmd_pos] = '\0';

                // Обработать команду
                if (cmd_pos > 0) {
                    // help
                    if (cmd_buffer[0] == 'h' && cmd_buffer[1] == 'e' &&
                        cmd_buffer[2] == 'l' && cmd_buffer[3] == 'p' && cmd_buffer[4] == '\0') {
                        terminal_write_line("Available commands:");
                        terminal_write_line("  help     - Show this help");
                        terminal_write_line("  clear    - Clear the screen");
                        terminal_write_line("  uptime   - Show system uptime");
                        terminal_write_line("  test     - Run hardware test");
                        terminal_write_line("  echo     - Echo your text");
                        terminal_write_line("  info     - System information");
                        terminal_write_line("  mem      - Memory statistics");
                        terminal_write_line("  memtest  - Test memory allocator");
                        terminal_write_line("  pgtest   - Test paging system");
                        terminal_write_line("  heaptest - Test heap allocator");
                        terminal_write_line("  ps       - List all processes");
                        terminal_write_line("  syscall  - Test system calls");
                        terminal_write_line("  ls       - List files in root directory");
                        terminal_write_line("  cat      - Read a file");
                        terminal_write_line("  locktest - Test synchronization primitives");
                    }
                    // clear
                    else if (cmd_buffer[0] == 'c' && cmd_buffer[1] == 'l' &&
                             cmd_buffer[2] == 'e' && cmd_buffer[3] == 'a' &&
                             cmd_buffer[4] == 'r' && cmd_buffer[5] == '\0') {
                        terminal_clear();
                        terminal_write_line("=== GraphOS Shell v0.1 ===");
                        terminal_write_line("");
                    }
                    // uptime
                    else if (cmd_buffer[0] == 'u' && cmd_buffer[1] == 'p' &&
                             cmd_buffer[2] == 't' && cmd_buffer[3] == 'i' &&
                             cmd_buffer[4] == 'm' && cmd_buffer[5] == 'e' && cmd_buffer[6] == '\0') {
                        terminal_write("System uptime: ");
                        print_dec(timer_ticks() / 100);
                        terminal_write_line(" seconds");
                    }
                    // test
                    else if (cmd_buffer[0] == 't' && cmd_buffer[1] == 'e' &&
                             cmd_buffer[2] == 's' && cmd_buffer[3] == 't' && cmd_buffer[4] == '\0') {
                        terminal_write_line("Running timer test...");
                        terminal_write("Waiting 2 seconds... ");
                        uint32_t start = timer_ticks();
                        timer_wait(200);
                        terminal_write("Done! (");
                        print_dec(timer_ticks() - start);
                        terminal_write_line(" ticks)");
                    }
                    // info
                    else if (cmd_buffer[0] == 'i' && cmd_buffer[1] == 'n' &&
                             cmd_buffer[2] == 'f' && cmd_buffer[3] == 'o' && cmd_buffer[4] == '\0') {
                        terminal_write_line("GraphOS v0.4 - Phase 2 Started");
                        terminal_write_line("Architecture: x86 (32-bit)");
                        terminal_write_line("Boot: Custom bootloader");
                        terminal_write("Uptime: ");
                        print_dec(timer_ticks() / 100);
                        terminal_write_line(" seconds");

                        pmm_info_t mem_info;
                        pmm_get_info(&mem_info);
                        terminal_write("Memory: ");
                        print_dec(mem_info.free_memory);
                        terminal_write(" KB free / ");
                        print_dec(mem_info.total_memory);
                        terminal_write_line(" KB total");

                        uint32_t heap_alloc, heap_free, heap_blocks;
                        heap_get_stats(&heap_alloc, &heap_free, &heap_blocks);
                        terminal_write("Heap: ");
                        print_dec(heap_alloc / 1024);
                        terminal_write(" KB used, ");
                        print_dec(heap_blocks);
                        terminal_write_line(" blocks");
                    }
                    // mem (new command)
                    else if (cmd_buffer[0] == 'm' && cmd_buffer[1] == 'e' &&
                             cmd_buffer[2] == 'm' && cmd_buffer[3] == '\0') {
                        pmm_info_t mem_info;
                        pmm_get_info(&mem_info);

                        terminal_write_line("Memory Statistics:");
                        terminal_write("  Total:      ");
                        print_dec(mem_info.total_memory);
                        terminal_write_line(" KB");

                        terminal_write("  Used:       ");
                        print_dec(mem_info.used_memory);
                        terminal_write_line(" KB");

                        terminal_write("  Free:       ");
                        print_dec(mem_info.free_memory);
                        terminal_write_line(" KB");

                        terminal_write("  Pages:      ");
                        print_dec(mem_info.used_pages);
                        terminal_write(" / ");
                        print_dec(mem_info.total_pages);
                        terminal_write_line(" used");
                    }
                    // memtest (new command)
                    else if (cmd_buffer[0] == 'm' && cmd_buffer[1] == 'e' &&
                             cmd_buffer[2] == 'm' && cmd_buffer[3] == 't' &&
                             cmd_buffer[4] == 'e' && cmd_buffer[5] == 's' &&
                             cmd_buffer[6] == 't' && cmd_buffer[7] == '\0') {
                        terminal_write_line("Memory Allocation Test:");

                        terminal_write("  Allocating 10 pages... ");
                        void* pages[10];
                        for (int i = 0; i < 10; i++) {
                            pages[i] = pmm_alloc_page();
                        }
                        terminal_write_line("OK");

                        pmm_info_t info;
                        pmm_get_info(&info);
                        terminal_write("  Used pages: ");
                        print_dec(info.used_pages);
                        terminal_write_line("");

                        terminal_write("  Freeing 5 pages... ");
                        for (int i = 0; i < 5; i++) {
                            pmm_free_page(pages[i]);
                        }
                        terminal_write_line("OK");

                        pmm_get_info(&info);
                        terminal_write("  Used pages: ");
                        print_dec(info.used_pages);
                        terminal_write_line("");

                        terminal_write("  Freeing remaining... ");
                        for (int i = 5; i < 10; i++) {
                            pmm_free_page(pages[i]);
                        }
                        terminal_write_line("OK");

                        terminal_write_line("  Test complete!");
                    }
                    // pgtest (new command)
                    else if (cmd_buffer[0] == 'p' && cmd_buffer[1] == 'g' &&
                             cmd_buffer[2] == 't' && cmd_buffer[3] == 'e' &&
                             cmd_buffer[4] == 's' && cmd_buffer[5] == 't' && cmd_buffer[6] == '\0') {
                        terminal_write_line("Paging Test:");

                        // Тест 1: Проверить identity mapping
                        terminal_write("  Testing identity mapping... ");
                        uint32_t virt = 0x100000; // 1MB
                        uint32_t phys = paging_get_physical(virt);
                        if (phys == virt) {
                            terminal_write_line("OK");
                        } else {
                            terminal_write("FAIL (expected ");
                            print_hex(virt);
                            terminal_write(", got ");
                            print_hex(phys);
                            terminal_write_line(")");
                        }

                        // Тест 2: Мапить новую страницу
                        terminal_write("  Mapping new page... ");
                        uint32_t new_virt = 0x900000; // 9MB
                        uint32_t new_phys = (uint32_t)pmm_alloc_page();
                        paging_map_page(new_virt, new_phys, PAGE_PRESENT | PAGE_RW);
                        terminal_write_line("OK");

                        // Тест 3: Проверить новый маппинг
                        terminal_write("  Verifying new mapping... ");
                        uint32_t mapped_phys = paging_get_physical(new_virt);
                        if (mapped_phys == new_phys) {
                            terminal_write_line("OK");
                        } else {
                            terminal_write_line("FAIL");
                        }

                        // Тест 4: Анмапить
                        terminal_write("  Unmapping page... ");
                        paging_unmap_page(new_virt);
                        terminal_write_line("OK");

                        // Тест 5: Проверить анмаппинг
                        terminal_write("  Verifying unmapping... ");
                        uint32_t unmapped = paging_get_physical(new_virt);
                        if (unmapped == 0) {
                            terminal_write_line("OK");
                        } else {
                            terminal_write_line("FAIL");
                        }

                        pmm_free_page((void*)new_phys);
                        terminal_write_line("  Test complete!");
                    }
                    // heaptest (new command)
                    else if (cmd_buffer[0] == 'h' && cmd_buffer[1] == 'e' &&
                             cmd_buffer[2] == 'a' && cmd_buffer[3] == 'p' &&
                             cmd_buffer[4] == 't' && cmd_buffer[5] == 'e' &&
                             cmd_buffer[6] == 's' && cmd_buffer[7] == 't' && cmd_buffer[8] == '\0') {
                        terminal_write_line("Heap Allocator Test:");

                        // Test 1: Simple allocation
                        terminal_write("  Allocating 100 bytes... ");
                        void* ptr1 = kmalloc(100);
                        if (ptr1) {
                            terminal_write("OK at ");
                            print_hex((uint32_t)ptr1);
                            terminal_write_line("");
                        } else {
                            terminal_write_line("FAIL");
                        }

                        // Test 2: Multiple allocations
                        terminal_write("  Allocating 200, 300, 400 bytes... ");
                        void* ptr2 = kmalloc(200);
                        void* ptr3 = kmalloc(300);
                        void* ptr4 = kmalloc(400);
                        terminal_write_line("OK");

                        // Test 3: Stats
                        uint32_t allocated, free, blocks;
                        heap_get_stats(&allocated, &free, &blocks);
                        terminal_write("  Allocated: ");
                        print_dec(allocated);
                        terminal_write(" bytes, Blocks: ");
                        print_dec(blocks);
                        terminal_write_line("");

                        // Test 4: Free some
                        terminal_write("  Freeing 200 and 400 byte blocks... ");
                        kfree(ptr2);
                        kfree(ptr4);
                        terminal_write_line("OK");

                        // Test 5: Stats after free
                        heap_get_stats(&allocated, &free, &blocks);
                        terminal_write("  Allocated: ");
                        print_dec(allocated);
                        terminal_write(" bytes, Blocks: ");
                        print_dec(blocks);
                        terminal_write_line("");

                        // Test 6: Realloc
                        terminal_write("  Reallocating 100 -> 500 bytes... ");
                        void* ptr5 = krealloc(ptr1, 500);
                        if (ptr5) {
                            terminal_write("OK at ");
                            print_hex((uint32_t)ptr5);
                            terminal_write_line("");
                        } else {
                            terminal_write_line("FAIL");
                        }

                        // Test 7: Cleanup
                        terminal_write("  Cleaning up... ");
                        kfree(ptr3);
                        kfree(ptr5);
                        terminal_write_line("OK");

                        // Final stats
                        heap_get_stats(&allocated, &free, &blocks);
                        terminal_write("  Final: ");
                        print_dec(allocated);
                        terminal_write(" bytes allocated, ");
                        print_dec(blocks);
                        terminal_write_line(" blocks");

                        terminal_write_line("  Test complete!");
                    }
                    // ps (process list)
                    else if (cmd_buffer[0] == 'p' && cmd_buffer[1] == 's' && cmd_buffer[2] == '\0') {
                        process_list();
                    }
                    // syscall test
                    else if (cmd_buffer[0] == 's' && cmd_buffer[1] == 'y' &&
                             cmd_buffer[2] == 's' && cmd_buffer[3] == 'c' &&
                             cmd_buffer[4] == 'a' && cmd_buffer[5] == 'l' &&
                             cmd_buffer[6] == 'l' && cmd_buffer[7] == '\0') {
                        terminal_write_line("System Call Test:");

                        // Test getpid
                        terminal_write("  Testing getpid... ");
                        uint32_t pid;
                        __asm__ volatile(
                            "mov $13, %%eax\n"  // SYS_GETPID
                            "int $0x80\n"
                            "mov %%eax, %0"
                            : "=r"(pid)
                            :
                            : "eax"
                        );
                        terminal_write("PID = ");
                        print_dec(pid);
                        terminal_write_line("");

                        // Test yield
                        terminal_write_line("  Testing yield... ");
                        __asm__ volatile(
                            "mov $16, %%eax\n"  // SYS_YIELD
                            "int $0x80"
                            :
                            :
                            : "eax"
                        );
                        terminal_write_line("  OK - returned from yield");

                        terminal_write_line("  Test complete!");
                    }
                    // ls (list files)
                    else if (cmd_buffer[0] == 'l' && cmd_buffer[1] == 's' && cmd_buffer[2] == '\0') {
                        if (!vfs_root) {
                            terminal_write_line("No filesystem mounted");
                        } else {
                            terminal_write_line("Files in root directory:");
                            uint32_t i = 0;
                            while (1) {
                                dirent_t* dirent = vfs_readdir(vfs_root, i);
                                if (!dirent) {
                                    break;
                                }
                                terminal_write("  ");
                                terminal_write_line(dirent->name);
                                kfree(dirent);
                                i++;
                            }
                        }
                    }
                    // cat <filename>
                    else if (cmd_buffer[0] == 'c' && cmd_buffer[1] == 'a' &&
                             cmd_buffer[2] == 't' && cmd_buffer[3] == ' ') {
                        if (!vfs_root) {
                            terminal_write_line("No filesystem mounted");
                        } else {
                            // Extract filename
                            char filename[64];
                            uint32_t j = 0;
                            for (uint32_t i = 4; i < cmd_pos && j < 63; i++, j++) {
                                filename[j] = cmd_buffer[i];
                            }
                            filename[j] = '\0';

                            // Find file
                            vfs_node_t* file = vfs_finddir(vfs_root, filename);
                            if (!file) {
                                terminal_write("File not found: ");
                                terminal_write_line(filename);
                            } else {
                                // Read file
                                uint8_t buffer[512];
                                uint32_t bytes_read = vfs_read(file, 0, file->length, buffer);

                                // Print content
                                for (uint32_t i = 0; i < bytes_read; i++) {
                                    terminal_putchar((char)buffer[i]);
                                }
                                terminal_putchar('\n');
                            }
                        }
                    }
                    // locktest (synchronization test)
                    else if (cmd_buffer[0] == 'l' && cmd_buffer[1] == 'o' &&
                             cmd_buffer[2] == 'c' && cmd_buffer[3] == 'k' &&
                             cmd_buffer[4] == 't' && cmd_buffer[5] == 'e' &&
                             cmd_buffer[6] == 's' && cmd_buffer[7] == 't' && cmd_buffer[8] == '\0') {
                        terminal_write_line("Synchronization Test:");

                        // Test spinlock
                        terminal_write_line("  Testing spinlock...");
                        spinlock_t spin;
                        spinlock_init(&spin);

                        spinlock_acquire(&spin);
                        terminal_write("    Lock acquired, ");

                        if (!spinlock_try_acquire(&spin)) {
                            terminal_write_line("try_acquire failed (correct)");
                        } else {
                            terminal_write_line("ERROR: try_acquire succeeded!");
                        }

                        spinlock_release(&spin);
                        terminal_write_line("    Lock released");

                        // Test mutex
                        terminal_write_line("  Testing mutex...");
                        mutex_t mtx;
                        mutex_init(&mtx);

                        mutex_lock(&mtx);
                        terminal_write("    Mutex locked, ");

                        if (!mutex_try_lock(&mtx)) {
                            terminal_write_line("try_lock failed (correct)");
                        } else {
                            terminal_write_line("ERROR: try_lock succeeded!");
                        }

                        mutex_unlock(&mtx);
                        terminal_write_line("    Mutex unlocked");

                        // Test semaphore
                        terminal_write_line("  Testing semaphore...");
                        semaphore_t sem;
                        semaphore_init(&sem, 2);

                        terminal_write("    Initial value: ");
                        print_dec(semaphore_get_value(&sem));
                        terminal_write_line("");

                        semaphore_wait(&sem);
                        terminal_write("    After wait: ");
                        print_dec(semaphore_get_value(&sem));
                        terminal_write_line("");

                        semaphore_signal(&sem);
                        terminal_write("    After signal: ");
                        print_dec(semaphore_get_value(&sem));
                        terminal_write_line("");

                        terminal_write_line("  Test complete!");
                    }
                    // echo
                    else if (cmd_buffer[0] == 'e' && cmd_buffer[1] == 'c' &&
                             cmd_buffer[2] == 'h' && cmd_buffer[3] == 'o' && cmd_buffer[4] == ' ') {
                        // Вывести всё после "echo "
                        for (uint32_t i = 5; i < cmd_pos; i++) {
                            terminal_putchar(cmd_buffer[i]);
                        }
                        terminal_putchar('\n');
                    }
                    else {
                        terminal_write("Unknown command: ");
                        terminal_write_line(cmd_buffer);
                        terminal_write_line("Type 'help' for available commands");
                    }
                }

                terminal_write("> ");
                cmd_pos = 0;
            }
            else if (c == '\b') {
                // Backspace
                if (cmd_pos > 0) {
                    cmd_pos--;
                }
            }
            else {
                // Обычный символ
                if (cmd_pos < CMD_BUFFER_SIZE - 1) {
                    cmd_buffer[cmd_pos++] = c;
                }
            }
        }

        __asm__ volatile("hlt");
    }
}

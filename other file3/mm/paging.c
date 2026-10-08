// ============================================================================
// Paging (Virtual Memory Management)
// Управление виртуальной памятью через page tables
// ============================================================================

#include "../include/paging.h"
#include "../include/pmm.h"

// Внешние функции
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void print_hex(uint32_t value);

// Глобальные переменные
page_directory_t* kernel_directory = 0;
page_directory_t* current_directory = 0;

// Внешняя функция из assembly для загрузки CR3
extern void paging_load_directory(uint32_t* dir);
extern void paging_enable_paging(void);

// ============================================================================
// Функция: paging_flush_tlb
// Сброс TLB для конкретного адреса
// ============================================================================
void paging_flush_tlb(uint32_t virt) {
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

// ============================================================================
// Функция: paging_map_page
// Мапит виртуальный адрес на физический
// ============================================================================
void paging_map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    // Получить индексы в page directory и page table
    uint32_t pd_index = virt >> 22;           // Старшие 10 бит
    uint32_t pt_index = (virt >> 12) & 0x3FF; // Средние 10 бит

    // Получить page directory entry
    page_directory_entry_t* pde = &current_directory->entries[pd_index];

    page_table_t* table;

    // Проверить, существует ли page table
    if (!pde->present) {
        // Создать новую page table
        uint32_t table_phys = (uint32_t)pmm_alloc_page();
        if (table_phys == 0) {
            terminal_write_line("[PAGING] ERROR: Cannot allocate page table!");
            return;
        }

        // Очистить таблицу
        table = (page_table_t*)table_phys;
        for (int i = 0; i < ENTRIES_PER_TABLE; i++) {
            table->entries[i].present = 0;
            table->entries[i].frame = 0;
        }

        // Установить page directory entry
        pde->present = 1;
        pde->rw = 1;
        pde->user = (flags & PAGE_USER) ? 1 : 0;
        pde->table = table_phys >> 12;
    } else {
        // Page table уже существует
        table = (page_table_t*)(pde->table << 12);
    }

    // Установить page table entry
    page_table_entry_t* pte = &table->entries[pt_index];
    pte->present = (flags & PAGE_PRESENT) ? 1 : 0;
    pte->rw = (flags & PAGE_RW) ? 1 : 0;
    pte->user = (flags & PAGE_USER) ? 1 : 0;
    pte->frame = phys >> 12;

    // Сброс TLB для этого адреса
    paging_flush_tlb(virt);
}

// ============================================================================
// Функция: paging_unmap_page
// Убирает маппинг виртуального адреса
// ============================================================================
void paging_unmap_page(uint32_t virt) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FF;

    page_directory_entry_t* pde = &current_directory->entries[pd_index];

    if (!pde->present) {
        return; // Page table не существует
    }

    page_table_t* table = (page_table_t*)(pde->table << 12);
    page_table_entry_t* pte = &table->entries[pt_index];

    if (pte->present) {
        pte->present = 0;
        pte->frame = 0;
        paging_flush_tlb(virt);
    }
}

// ============================================================================
// Функция: paging_get_physical
// Получает физический адрес для виртуального
// ============================================================================
uint32_t paging_get_physical(uint32_t virt) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FF;
    uint32_t offset = virt & 0xFFF;

    page_directory_entry_t* pde = &current_directory->entries[pd_index];

    if (!pde->present) {
        return 0; // Not mapped
    }

    page_table_t* table = (page_table_t*)(pde->table << 12);
    page_table_entry_t* pte = &table->entries[pt_index];

    if (!pte->present) {
        return 0; // Not mapped
    }

    return (pte->frame << 12) | offset;
}

// ============================================================================
// Функция: paging_switch_directory
// Переключает page directory
// ============================================================================
void paging_switch_directory(page_directory_t* dir) {
    current_directory = dir;
    __asm__ volatile("mov %0, %%cr3" : : "r"((uint32_t)dir));
}

// ============================================================================
// ============================================================================
// Функция: page_fault_handler
// Обработчик page fault (ISR 14)
// ============================================================================
void page_fault_handler(uint32_t error_code, uint32_t faulting_address) {
    terminal_write_line("");
    terminal_write_line("*** PAGE FAULT ***");

    terminal_write("Faulting address: ");
    print_hex(faulting_address);
    terminal_write_line("");

    terminal_write("Error code: ");
    print_hex(error_code);
    terminal_write_line("");

    terminal_write("  ");
    if (!(error_code & 0x1)) terminal_write("Page not present ");
    if (error_code & 0x2) terminal_write("Write access ");
    else terminal_write("Read access ");
    if (error_code & 0x4) terminal_write("(User mode) ");
    else terminal_write("(Kernel mode) ");
    terminal_write_line("");

    terminal_write_line("*** HALTING SYSTEM ***");
    for(;;) __asm__ volatile("hlt");
}

// ============================================================================
// Функция: paging_init
// Инициализирует paging с identity mapping для kernel
// ============================================================================
void paging_init(void) {
    terminal_write_line("[PAGING] Initializing virtual memory...");

    // Выделить page directory
    kernel_directory = (page_directory_t*)pmm_alloc_page();
    if ((uint32_t)kernel_directory == 0) {
        terminal_write_line("[PAGING] ERROR: Cannot allocate page directory!");
        return;
    }

    // Очистить page directory
    for (int i = 0; i < ENTRIES_PER_DIRECTORY; i++) {
        kernel_directory->entries[i].present = 0;
        kernel_directory->entries[i].table = 0;
    }

    current_directory = kernel_directory;

    terminal_write_line("[PAGING] Creating identity mapping (0-8MB)...");

    // Identity mapping для первых 8MB
    // 0x000000 - 0x7FFFFF (VGA, BIOS, kernel, initial heap)
    for (uint32_t addr = 0; addr < 0x800000; addr += PAGE_SIZE) {
        paging_map_page(addr, addr, PAGE_PRESENT | PAGE_RW);
    }

    terminal_write_line("[PAGING] Identity mapping complete");

    // Регистрировать page fault handler (будет добавлено позже)
    terminal_write_line("[PAGING] Registering page fault handler...");
    // register_interrupt_handler(14, page_fault_handler);

    terminal_write_line("[OK] Paging initialized (not enabled yet)");
}

// ============================================================================
// Функция: paging_enable
// Включает paging через CR0
// ============================================================================
void paging_enable(void) {
    terminal_write_line("[PAGING] Enabling paging...");

    // Загрузить page directory в CR3
    __asm__ volatile("mov %0, %%cr3" : : "r"(kernel_directory));

    // Включить paging через CR0
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Set PG bit
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));

    terminal_write_line("[OK] Paging enabled!");
}

// ============================================================================
// Функция: paging_clone_directory
// Клонирует page directory (для процессов)
// ============================================================================
page_directory_t* paging_clone_directory(page_directory_t* src) {
    // Выделить новый page directory
    uint32_t phys = (uint32_t)pmm_alloc_page();
    if (phys == 0) {
        return 0;
    }

    page_directory_t* dir = (page_directory_t*)phys;

    // Скопировать записи
    for (int i = 0; i < ENTRIES_PER_DIRECTORY; i++) {
        if (!src->entries[i].present) {
            dir->entries[i].present = 0;
        } else {
            // Клонировать page table
            uint32_t table_phys = (uint32_t)pmm_alloc_page();
            page_table_t* table = (page_table_t*)table_phys;
            page_table_t* src_table = (page_table_t*)(src->entries[i].table << 12);

            // Копировать записи page table
            for (int j = 0; j < ENTRIES_PER_TABLE; j++) {
                table->entries[j] = src_table->entries[j];
            }

            dir->entries[i] = src->entries[i];
            dir->entries[i].table = table_phys >> 12;
        }
    }

    return dir;
}

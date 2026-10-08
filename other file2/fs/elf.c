// ============================================================================
// ELF Loader Implementation
// Загрузка и выполнение ELF32 исполняемых файлов
// ============================================================================

#include "../include/elf.h"
#include "../include/vfs.h"
#include "../include/process.h"
#include "../include/paging.h"
#include "../include/heap.h"
#include <stdint.h>
#include <stddef.h>

// Внешние функции вывода (для отладки)
extern void terminal_write_line(const char* str);
extern void terminal_write(const char* str);
extern void terminal_write_hex(uint32_t value);

// ============================================================================
// Validate ELF Header
// ============================================================================

int elf_validate(elf32_header_t* header) {
    // Проверить magic number
    if (header->e_ident[0] != 0x7F ||
        header->e_ident[1] != 'E' ||
        header->e_ident[2] != 'L' ||
        header->e_ident[3] != 'F') {
        return -1;  // Не ELF файл
    }

    // Проверить класс (32-bit)
    if (header->e_ident[4] != 1) {
        return -1;  // Не 32-bit
    }

    // Проверить endianness (little-endian)
    if (header->e_ident[5] != 1) {
        return -1;  // Не little-endian
    }

    // Проверить тип (executable)
    if (header->e_type != ET_EXEC) {
        return -1;  // Не исполняемый файл
    }

    // Проверить архитектуру (x86)
    if (header->e_machine != EM_386) {
        return -1;  // Не x86
    }

    return 0;  // Валидный ELF
}

// ============================================================================
// Load ELF File
// ============================================================================

int elf_load(const char* path, int argc, char** argv) {
    terminal_write("Loading ELF: ");
    terminal_write_line(path);

    // Открыть файл
    const char* fname = (path[0] == '/') ? (path + 1) : path;
    vfs_node_t* file = vfs_finddir(vfs_root, fname);
    if (file) vfs_open(file, 0);
    if (!file) {
        terminal_write_line("  [ERROR] File not found");
        return -1;
    }

    // Прочитать ELF заголовок
    elf32_header_t header;
    uint32_t read = vfs_read(file, 0, sizeof(elf32_header_t), (uint8_t*)&header);
    if (read != sizeof(elf32_header_t)) {
        terminal_write_line("  [ERROR] Cannot read ELF header");
        vfs_close(file);
        return -1;
    }

    // Валидировать заголовок
    if (elf_validate(&header) != 0) {
        terminal_write_line("  [ERROR] Invalid ELF file");
        vfs_close(file);
        return -1;
    }

    terminal_write("  Entry point: 0x");
    terminal_write_hex(header.e_entry);
    terminal_write_line("");

    // Создать новый процесс для программы (приоритет 1 - нормальный)
    process_t* proc = process_create(path, (void (*)(void))header.e_entry, 1);
    if (!proc) {
        terminal_write_line("  [ERROR] Cannot create process");
        vfs_close(file);
        return -1;
    }

    // Загрузить program headers
    for (uint16_t i = 0; i < header.e_phnum; i++) {
        elf32_program_header_t phdr;
        uint32_t offset = header.e_phoff + i * header.e_phentsize;

        read = vfs_read(file, offset, sizeof(elf32_program_header_t), (uint8_t*)&phdr);
        if (read != sizeof(elf32_program_header_t)) {
            terminal_write_line("  [ERROR] Cannot read program header");
            vfs_close(file);
            return -1;
        }

        // Загружать только LOAD сегменты
        if (phdr.p_type != PT_LOAD) {
            continue;
        }

        terminal_write("  Loading segment: vaddr=0x");
        terminal_write_hex(phdr.p_vaddr);
        terminal_write(" size=");
        terminal_write_hex(phdr.p_memsz);
        terminal_write_line("");

        // Выделить память для сегмента
        uint32_t page_count = (phdr.p_memsz + 0xFFF) / 0x1000;
        for (uint32_t j = 0; j < page_count; j++) {
            uint32_t virt_addr = phdr.p_vaddr + j * 0x1000;

            // Выделить физическую страницу
            extern uint32_t pmm_alloc_page(void);
            uint32_t phys_addr = pmm_alloc_page();

            // Замапить в адресное пространство процесса
            uint32_t flags = 0x07;  // Present, R/W, User
            paging_map_page(virt_addr, phys_addr, flags);
        }

        // Загрузить данные сегмента
        if (phdr.p_filesz > 0) {
            uint8_t* dest = (uint8_t*)phdr.p_vaddr;
            read = vfs_read(file, phdr.p_offset, phdr.p_filesz, dest);
            if (read != phdr.p_filesz) {
                terminal_write_line("  [ERROR] Cannot read segment data");
                vfs_close(file);
                return -1;
            }
        }

        // Очистить BSS (если memsz > filesz)
        if (phdr.p_memsz > phdr.p_filesz) {
            uint8_t* bss_start = (uint8_t*)(phdr.p_vaddr + phdr.p_filesz);
            uint32_t bss_size = phdr.p_memsz - phdr.p_filesz;
            for (uint32_t j = 0; j < bss_size; j++) {
                bss_start[j] = 0;
            }
        }
    }

    vfs_close(file);

    terminal_write_line("  [OK] ELF loaded successfully");

    // Добавить процесс в scheduler
    scheduler_add_process(proc);

    return 0;  // Успешно загружен
}

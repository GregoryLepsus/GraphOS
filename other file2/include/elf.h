#ifndef ELF_H
#define ELF_H

#include <stdint.h>

// ELF32 Magic Number
#define ELF_MAGIC 0x464C457F  // "\x7FELF"

// ELF Types
#define ET_NONE   0  // No file type
#define ET_REL    1  // Relocatable file
#define ET_EXEC   2  // Executable file
#define ET_DYN    3  // Shared object file
#define ET_CORE   4  // Core file

// ELF Machine Types
#define EM_NONE   0  // No machine
#define EM_386    3  // Intel 80386
#define EM_X86_64 62 // AMD x86-64

// ELF Program Header Types
#define PT_NULL    0  // Unused entry
#define PT_LOAD    1  // Loadable segment
#define PT_DYNAMIC 2  // Dynamic linking info
#define PT_INTERP  3  // Interpreter path
#define PT_NOTE    4  // Auxiliary info

// ELF Section Header Types
#define SHT_NULL     0  // Unused
#define SHT_PROGBITS 1  // Program data
#define SHT_SYMTAB   2  // Symbol table
#define SHT_STRTAB   3  // String table
#define SHT_NOBITS   8  // BSS (uninitialized data)

// Program Header Flags
#define PF_X 0x1  // Execute
#define PF_W 0x2  // Write
#define PF_R 0x4  // Read

// ELF32 Header
typedef struct {
    uint8_t  e_ident[16];     // Magic number and other info
    uint16_t e_type;          // Object file type
    uint16_t e_machine;       // Architecture
    uint32_t e_version;       // Object file version
    uint32_t e_entry;         // Entry point virtual address
    uint32_t e_phoff;         // Program header table offset
    uint32_t e_shoff;         // Section header table offset
    uint32_t e_flags;         // Processor-specific flags
    uint16_t e_ehsize;        // ELF header size
    uint16_t e_phentsize;     // Program header entry size
    uint16_t e_phnum;         // Program header entry count
    uint16_t e_shentsize;     // Section header entry size
    uint16_t e_shnum;         // Section header entry count
    uint16_t e_shstrndx;      // Section header string table index
} __attribute__((packed)) elf32_header_t;

// ELF32 Program Header
typedef struct {
    uint32_t p_type;          // Segment type
    uint32_t p_offset;        // Segment file offset
    uint32_t p_vaddr;         // Segment virtual address
    uint32_t p_paddr;         // Segment physical address
    uint32_t p_filesz;        // Segment size in file
    uint32_t p_memsz;         // Segment size in memory
    uint32_t p_flags;         // Segment flags
    uint32_t p_align;         // Segment alignment
} __attribute__((packed)) elf32_program_header_t;

// ELF32 Section Header
typedef struct {
    uint32_t sh_name;         // Section name (string table index)
    uint32_t sh_type;         // Section type
    uint32_t sh_flags;        // Section flags
    uint32_t sh_addr;         // Section virtual address
    uint32_t sh_offset;       // Section file offset
    uint32_t sh_size;         // Section size
    uint32_t sh_link;         // Link to another section
    uint32_t sh_info;         // Additional section info
    uint32_t sh_addralign;    // Section alignment
    uint32_t sh_entsize;      // Entry size if section holds table
} __attribute__((packed)) elf32_section_header_t;

// Загрузить и запустить ELF исполняемый файл
int elf_load(const char* path, int argc, char** argv);

// Проверить валидность ELF заголовка
int elf_validate(elf32_header_t* header);

#endif // ELF_H

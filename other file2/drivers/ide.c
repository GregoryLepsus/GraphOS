// ============================================================================
// IDE/ATA Driver
// Драйвер для чтения/записи на IDE диски
// ============================================================================

#include "../include/ide.h"
#include "../include/types.h"

// Внешние функции
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void print_hex(uint32_t value);

// I/O port functions
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

// ============================================================================
// Функция: ide_wait_ready
// Ожидает, пока диск будет готов
// ============================================================================
void ide_wait_ready(void) {
    while (inb(IDE_STATUS) & IDE_STATUS_BSY) {
        // Wait for BSY to clear
    }
}

// ============================================================================
// Функция: ide_poll
// Ожидает завершения операции
// ============================================================================
void ide_poll(void) {
    // Wait for BSY to clear
    for (int i = 0; i < 4; i++) {
        inb(IDE_STATUS);  // 400ns delay
    }

    uint8_t status;
    while (1) {
        status = inb(IDE_STATUS);
        if (!(status & IDE_STATUS_BSY)) {
            break;
        }
    }

    // Wait for DRQ or ERR
    while (1) {
        status = inb(IDE_STATUS);
        if (status & IDE_STATUS_DRQ) {
            break;
        }
        if (status & IDE_STATUS_ERR) {
            terminal_write_line("[IDE] ERROR during poll!");
            break;
        }
    }
}

// ============================================================================
// Функция: ide_init
// Инициализирует IDE драйвер
// ============================================================================
void ide_init(void) {
    terminal_write_line("[IDE] Initializing IDE driver...");

    // Select master drive
    outb(IDE_DRIVE, 0xA0);

    // Wait for drive to be ready
    ide_wait_ready();

    // Send IDENTIFY command
    outb(IDE_COMMAND, IDE_CMD_IDENTIFY);

    // Poll for completion
    uint8_t status = inb(IDE_STATUS);
    if (status == 0) {
        terminal_write_line("[IDE] No drive detected");
        return;
    }

    ide_poll();

    // Read identification data (256 words)
    uint16_t buffer[256];
    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(IDE_DATA);
    }

    terminal_write_line("[OK] IDE driver initialized");
}

// ============================================================================
// Функция: ide_read_sectors
// Читает сектора с диска
// ============================================================================
void ide_read_sectors(uint32_t lba, uint8_t count, uint16_t* buffer) {
    // Wait for drive to be ready
    ide_wait_ready();

    // Select master drive, LBA mode
    outb(IDE_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

    // Send sector count
    outb(IDE_SECTOR_CNT, count);

    // Send LBA
    outb(IDE_LBA_LOW, (uint8_t)(lba));
    outb(IDE_LBA_MID, (uint8_t)(lba >> 8));
    outb(IDE_LBA_HIGH, (uint8_t)(lba >> 16));

    // Send READ command
    outb(IDE_COMMAND, IDE_CMD_READ_SECTORS);

    // Read sectors
    for (int s = 0; s < count; s++) {
        ide_poll();

        // Read 256 words (512 bytes per sector)
        for (int i = 0; i < 256; i++) {
            buffer[s * 256 + i] = inw(IDE_DATA);
        }
    }
}

// ============================================================================
// Функция: ide_write_sectors
// Записывает сектора на диск
// ============================================================================
void ide_write_sectors(uint32_t lba, uint8_t count, uint16_t* buffer) {
    // Wait for drive to be ready
    ide_wait_ready();

    // Select master drive, LBA mode
    outb(IDE_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

    // Send sector count
    outb(IDE_SECTOR_CNT, count);

    // Send LBA
    outb(IDE_LBA_LOW, (uint8_t)(lba));
    outb(IDE_LBA_MID, (uint8_t)(lba >> 8));
    outb(IDE_LBA_HIGH, (uint8_t)(lba >> 16));

    // Send WRITE command
    outb(IDE_COMMAND, IDE_CMD_WRITE_SECTORS);

    // Write sectors
    for (int s = 0; s < count; s++) {
        ide_poll();

        // Write 256 words (512 bytes per sector)
        for (int i = 0; i < 256; i++) {
            outw(IDE_DATA, buffer[s * 256 + i]);
        }
    }

    // Flush cache
    outb(IDE_COMMAND, 0xE7);  // CACHE FLUSH command
    ide_wait_ready();
}

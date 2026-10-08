// ============================================================================
// ACPI (Advanced Configuration and Power Interface)
// Basic ACPI support for power management
// ============================================================================

#include "../include/acpi.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// Port I/O
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// ACPI global info
static acpi_info_t acpi_info;
static int acpi_available = 0;

// ============================================================================
// Memory Operations
// ============================================================================

static int memcmp(const void* s1, const void* s2, int n) {
    const uint8_t* p1 = s1;
    const uint8_t* p2 = s2;
    for (int i = 0; i < n; i++) {
        if (p1[i] != p2[i]) return p1[i] - p2[i];
    }
    return 0;
}

// ============================================================================
// RSDP Detection
// ============================================================================

static acpi_rsdp_t* acpi_find_rsdp_in_range(uint32_t start, uint32_t end) {
    for (uint32_t addr = start; addr < end; addr += 16) {
        acpi_rsdp_t* rsdp = (acpi_rsdp_t*)addr;
        if (memcmp(rsdp->signature, "RSD PTR ", 8) == 0) {
            // Verify checksum
            uint8_t sum = 0;
            uint8_t* ptr = (uint8_t*)rsdp;
            for (int i = 0; i < 20; i++) {
                sum += ptr[i];
            }
            if (sum == 0) {
                return rsdp;
            }
        }
    }
    return NULL;
}

int acpi_find_rsdp(void) {
    // Search EBDA (Extended BIOS Data Area)
    uint32_t ebda = *(uint16_t*)0x40E;
    ebda <<= 4;

    acpi_rsdp_t* rsdp = acpi_find_rsdp_in_range(ebda, ebda + 1024);

    if (!rsdp) {
        // Search BIOS ROM area
        rsdp = acpi_find_rsdp_in_range(0xE0000, 0x100000);
    }

    if (rsdp) {
        acpi_info.rsdp_address = (uint64_t)rsdp;
        acpi_info.rsdt_address = rsdp->rsdt_address;
        acpi_info.acpi_version = rsdp->revision;
        return 1;
    }

    return 0;
}

// ============================================================================
// Initialization
// ============================================================================

void acpi_init(void) {
    terminal_write_line("[ACPI] Initializing ACPI...");

    if (acpi_find_rsdp()) {
        acpi_available = 1;
        terminal_write("[ACPI] RSDP found at: 0x");
        terminal_write_hex((uint32_t)acpi_info.rsdp_address);
        terminal_write_line("");
        terminal_write("[ACPI] ACPI Version: ");
        terminal_write_hex(acpi_info.acpi_version);
        terminal_write_line("");
    } else {
        terminal_write_line("[ACPI] RSDP not found - ACPI unavailable");
        acpi_available = 0;
    }
}

// ============================================================================
// Get ACPI Info
// ============================================================================

void acpi_get_info(acpi_info_t* info) {
    if (info) {
        info->rsdp_address = acpi_info.rsdp_address;
        info->rsdt_address = acpi_info.rsdt_address;
        info->acpi_version = acpi_info.acpi_version;
    }
}

// ============================================================================
// Power Management
// ============================================================================

int acpi_shutdown(void) {
    terminal_write_line("[ACPI] System shutdown requested");

    // Try ACPI shutdown (simplified - would need FADT parsing)
    // For now, use keyboard controller method
    outb(0x64, 0xFE);  // Pulse reset line

    // Alternative: Try APM
    outw(0xB004, 0x2000);  // Bochs/QEMU shutdown
    outw(0x604, 0x2000);   // QEMU shutdown port
    outw(0x4004, 0x3400);  // VirtualBox shutdown

    return 0;
}

int acpi_reboot(void) {
    terminal_write_line("[ACPI] System reboot requested");

    // Use keyboard controller to reboot
    uint8_t temp;

    // Disable interrupts
    __asm__ volatile("cli");

    // Clear keyboard buffer
    do {
        temp = inb(0x64);
        if (temp & 0x01) {
            inb(0x60);
        }
    } while (temp & 0x02);

    // Pulse reset line
    outb(0x64, 0xFE);

    // Halt if reset fails
    __asm__ volatile("hlt");

    return 0;
}

int acpi_sleep(acpi_state_t state) {
    // Sleep states not fully implemented yet
    terminal_write("[ACPI] Sleep state S");
    terminal_write_hex((uint8_t)state);
    terminal_write_line(" not implemented");
    return -1;
}

int acpi_enable(void) {
    if (!acpi_available) {
        return -1;
    }

    // Enable ACPI (simplified)
    return 0;
}

// ============================================================================
// AHCI SATA Driver
// Modern SATA storage interface
// ============================================================================

#include "../include/ahci.h"
#include "../include/pci.h"
#include "../include/heap.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// AHCI HBA Memory Registers
typedef struct {
    uint32_t cap;        // Host capabilities
    uint32_t ghc;        // Global host control
    uint32_t is;         // Interrupt status
    uint32_t pi;         // Ports implemented
    uint32_t vs;         // Version
    uint32_t ccc_ctl;    // Command completion coalescing control
    uint32_t ccc_pts;    // Command completion coalescing ports
    uint32_t em_loc;     // Enclosure management location
    uint32_t em_ctl;     // Enclosure management control
    uint32_t cap2;       // Host capabilities extended
    uint32_t bohc;       // BIOS/OS handoff control and status
} __attribute__((packed)) ahci_hba_mem_t;

// AHCI Port Registers
typedef struct {
    uint32_t clb;        // Command list base address
    uint32_t clbu;       // Command list base address upper 32 bits
    uint32_t fb;         // FIS base address
    uint32_t fbu;        // FIS base address upper 32 bits
    uint32_t is;         // Interrupt status
    uint32_t ie;         // Interrupt enable
    uint32_t cmd;        // Command and status
    uint32_t rsv0;       // Reserved
    uint32_t tfd;        // Task file data
    uint32_t sig;        // Signature
    uint32_t ssts;       // SATA status
    uint32_t sctl;       // SATA control
    uint32_t serr;       // SATA error
    uint32_t sact;       // SATA active
    uint32_t ci;         // Command issue
    uint32_t sntf;       // SATA notification
    uint32_t fbs;        // FIS-based switch control
} __attribute__((packed)) ahci_port_t;

// AHCI Command Header
typedef struct {
    uint8_t cfl:5;       // Command FIS length
    uint8_t a:1;         // ATAPI
    uint8_t w:1;         // Write
    uint8_t p:1;         // Prefetchable
    uint8_t r:1;         // Reset
    uint8_t b:1;         // BIST
    uint8_t c:1;         // Clear busy upon R_OK
    uint8_t rsv0:1;
    uint8_t pmp:4;       // Port multiplier port
    uint16_t prdtl;      // Physical region descriptor table length
    uint32_t prdbc;      // Physical region descriptor byte count
    uint32_t ctba;       // Command table descriptor base address
    uint32_t ctbau;      // Command table descriptor base address upper
    uint32_t rsv1[4];    // Reserved
} __attribute__((packed)) ahci_cmd_header_t;

// Global AHCI state
static ahci_hba_mem_t* abar = NULL;
static ahci_device_t devices[32];
static int device_count = 0;

// ============================================================================
// Port Management
// ============================================================================

static int ahci_port_check_type(ahci_port_t* port) {
    uint32_t ssts = port->ssts;
    uint8_t det = ssts & 0x0F;
    uint8_t ipm = (ssts >> 8) & 0x0F;

    if (det != 3) return 0;  // No device
    if (ipm != 1) return 0;  // Device not active

    return 1;
}

static void ahci_port_rebase(ahci_port_t* port, int portno) {
    // Stop command engine
    port->cmd &= ~0x0001;
    while (port->cmd & 0x4000);  // Wait for CR

    // Allocate command list
    uint32_t clb = (uint32_t)kmalloc(1024);
    port->clb = clb;
    port->clbu = 0;

    // Allocate FIS
    uint32_t fb = (uint32_t)kmalloc(256);
    port->fb = fb;
    port->fbu = 0;

    // Clear
    for (int i = 0; i < 1024; i += 4) {
        *(uint32_t*)(clb + i) = 0;
    }
    for (int i = 0; i < 256; i += 4) {
        *(uint32_t*)(fb + i) = 0;
    }

    // Setup command table
    ahci_cmd_header_t* cmdheader = (ahci_cmd_header_t*)clb;
    for (int i = 0; i < 32; i++) {
        cmdheader[i].prdtl = 8;
        uint32_t ctba = (uint32_t)kmalloc(256);
        cmdheader[i].ctba = ctba;
        cmdheader[i].ctbau = 0;
    }

    // Start command engine
    port->cmd |= 0x0001;
}

// ============================================================================
// Initialization
// ============================================================================

void ahci_init(void) {
    terminal_write_line("[AHCI] Initializing AHCI SATA...");

    // Find AHCI controller on PCI
    pci_device_t* ahci_dev = pci_find_class(0x01, 0x06);  // Mass storage, SATA

    if (!ahci_dev) {
        terminal_write_line("[AHCI] AHCI controller not found");
        return;
    }

    terminal_write("[AHCI] Found AHCI controller at PCI ");
    terminal_write_hex(ahci_dev->bus);
    terminal_write(":");
    terminal_write_hex(ahci_dev->device);
    terminal_write_line("");

    // Get ABAR (AHCI Base Address Register)
    uint32_t abar_addr = pci_read_bar(ahci_dev, 5);
    abar = (ahci_hba_mem_t*)abar_addr;

    terminal_write("[AHCI] ABAR: 0x");
    terminal_write_hex(abar_addr);
    terminal_write_line("");

    // Enable AHCI
    abar->ghc |= (1 << 31);

    terminal_write("[AHCI] Version: 0x");
    terminal_write_hex(abar->vs);
    terminal_write_line("");

    device_count = 0;
}

// ============================================================================
// Port Probing
// ============================================================================

int ahci_probe_ports(void) {
    if (!abar) {
        return 0;
    }

    uint32_t pi = abar->pi;
    device_count = 0;

    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) {
            ahci_port_t* port = (ahci_port_t*)((uint32_t)abar + 0x100 + (i * 0x80));

            if (ahci_port_check_type(port)) {
                terminal_write("[AHCI] SATA device found on port ");
                terminal_write_hex(i);
                terminal_write_line("");

                ahci_port_rebase(port, i);

                devices[device_count].port_num = i;
                devices[device_count].present = 1;
                devices[device_count].signature = port->sig;
                device_count++;
            }
        }
    }

    terminal_write("[AHCI] Found ");
    terminal_write_hex(device_count);
    terminal_write_line(" SATA devices");

    return device_count;
}

// ============================================================================
// Device Access
// ============================================================================

int ahci_read_sectors(uint8_t port, uint64_t lba, uint16_t count, void* buffer) {
    if (!abar || port >= 32) {
        return -1;
    }

    // Simplified implementation
    // Full implementation would setup FIS and PRD tables
    return 0;
}

int ahci_write_sectors(uint8_t port, uint64_t lba, uint16_t count, void* buffer) {
    if (!abar || port >= 32) {
        return -1;
    }

    // Simplified implementation
    return 0;
}

int ahci_identify_device(uint8_t port, ahci_device_t* device) {
    if (!abar || port >= 32 || !device) {
        return -1;
    }

    // Copy device info
    for (int i = 0; i < device_count; i++) {
        if (devices[i].port_num == port) {
            device->port_num = devices[i].port_num;
            device->present = devices[i].present;
            device->signature = devices[i].signature;
            device->capacity_sectors = 0;  // Would read from IDENTIFY
            device->sector_size = 512;
            return 0;
        }
    }

    return -1;
}

// ============================================================================
// Device Query
// ============================================================================

ahci_device_t* ahci_get_device(uint8_t port) {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].port_num == port) {
            return &devices[i];
        }
    }
    return NULL;
}

int ahci_get_port_count(void) {
    return device_count;
}

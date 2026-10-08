// ============================================================================
// PCI Driver - PCI Bus Enumeration and Configuration
// Обнаружение и настройка PCI устройств
// ============================================================================

#include "../include/pci.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// Список обнаруженных устройств
#define MAX_PCI_DEVICES 32
static pci_device_t pci_devices[MAX_PCI_DEVICES];
static int pci_device_count = 0;

// ============================================================================
// PCI Configuration Space Access
// ============================================================================

uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg) {
    uint32_t address = 0x80000000 |
                       ((uint32_t)bus << 16) |
                       ((uint32_t)device << 11) |
                       ((uint32_t)func << 8) |
                       (reg & 0xFC);

    // Write address
    __asm__ volatile("outl %0, %w1" : : "a"(address), "d"((uint16_t)PCI_CONFIG_ADDRESS));

    // Read data
    uint32_t data;
    __asm__ volatile("inl %w1, %0" : "=a"(data) : "d"((uint16_t)PCI_CONFIG_DATA));

    return data;
}

void pci_write(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg, uint32_t value) {
    uint32_t address = 0x80000000 |
                       ((uint32_t)bus << 16) |
                       ((uint32_t)device << 11) |
                       ((uint32_t)func << 8) |
                       (reg & 0xFC);

    // Write address
    __asm__ volatile("outl %0, %w1" : : "a"(address), "d"((uint16_t)PCI_CONFIG_ADDRESS));

    // Write data
    __asm__ volatile("outl %0, %w1" : : "a"(value), "d"((uint16_t)PCI_CONFIG_DATA));
}

uint16_t pci_read_word(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg) {
    uint32_t data = pci_read(bus, device, func, reg & 0xFC);
    return (data >> ((reg & 2) * 8)) & 0xFFFF;
}

uint8_t pci_read_byte(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg) {
    uint32_t data = pci_read(bus, device, func, reg & 0xFC);
    return (data >> ((reg & 3) * 8)) & 0xFF;
}

// ============================================================================
// PCI Device Functions
// ============================================================================

static void pci_check_function(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t vendor_id = pci_read_word(bus, device, function, PCI_VENDOR_ID);

    // Invalid vendor ID
    if (vendor_id == 0xFFFF) {
        return;
    }

    // Read device info
    if (pci_device_count < MAX_PCI_DEVICES) {
        pci_device_t* dev = &pci_devices[pci_device_count++];

        dev->vendor_id = vendor_id;
        dev->device_id = pci_read_word(bus, device, function, PCI_DEVICE_ID);
        dev->bus = bus;
        dev->device = device;
        dev->function = function;
        dev->class_code = pci_read_byte(bus, device, function, PCI_CLASS_CODE);
        dev->subclass = pci_read_byte(bus, device, function, PCI_SUBCLASS);
        dev->prog_if = pci_read_byte(bus, device, function, PCI_PROG_IF);
        dev->revision = pci_read_byte(bus, device, function, PCI_REVISION_ID);
        dev->interrupt_line = pci_read_byte(bus, device, function, PCI_INTERRUPT_LINE);

        // Read BARs
        for (int i = 0; i < 6; i++) {
            dev->bar[i] = pci_read(bus, device, function, PCI_BAR0 + i * 4);
        }
    }
}

static void pci_check_device(uint8_t bus, uint8_t device) {
    uint16_t vendor_id = pci_read_word(bus, device, 0, PCI_VENDOR_ID);

    if (vendor_id == 0xFFFF) {
        return;  // Device doesn't exist
    }

    pci_check_function(bus, device, 0);

    // Check for multi-function device
    uint8_t header_type = pci_read_byte(bus, device, 0, PCI_HEADER_TYPE);
    if (header_type & 0x80) {
        // Multi-function device
        for (uint8_t func = 1; func < 8; func++) {
            vendor_id = pci_read_word(bus, device, func, PCI_VENDOR_ID);
            if (vendor_id != 0xFFFF) {
                pci_check_function(bus, device, func);
            }
        }
    }
}

static void pci_scan_bus(uint8_t bus) {
    for (uint8_t device = 0; device < 32; device++) {
        pci_check_device(bus, device);
    }
}

// ============================================================================
// PCI Initialization
// ============================================================================

void pci_init(void) {
    terminal_write_line("[PCI] Scanning PCI bus...");

    pci_device_count = 0;

    // Scan bus 0
    pci_scan_bus(0);

    terminal_write("[PCI] Found ");
    terminal_write_hex(pci_device_count);
    terminal_write_line(" devices");

    // Print found devices
    for (int i = 0; i < pci_device_count; i++) {
        pci_device_t* dev = &pci_devices[i];
        terminal_write("[PCI] ");
        terminal_write_hex(dev->vendor_id);
        terminal_write(":");
        terminal_write_hex(dev->device_id);
        terminal_write(" Class: ");
        terminal_write_hex(dev->class_code);
        terminal_write_line("");
    }
}

// ============================================================================
// Device Search
// ============================================================================

pci_device_t* pci_find_device(uint16_t vendor_id, uint16_t device_id) {
    for (int i = 0; i < pci_device_count; i++) {
        if (pci_devices[i].vendor_id == vendor_id &&
            pci_devices[i].device_id == device_id) {
            return &pci_devices[i];
        }
    }
    return NULL;
}

// ============================================================================
// Enable Bus Mastering
// ============================================================================

void pci_enable_bus_mastering(pci_device_t* dev) {
    uint32_t command = pci_read(dev->bus, dev->device, dev->function, PCI_COMMAND);
    command |= PCI_COMMAND_BUS_MASTER | PCI_COMMAND_MEMORY;
    pci_write(dev->bus, dev->device, dev->function, PCI_COMMAND, command);
}

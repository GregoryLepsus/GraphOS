// ============================================================================
// E1000 Network Driver
// Intel 82540EM Gigabit Ethernet Controller Driver
// ============================================================================

#include "../include/e1000.h"
#include "../include/pci.h"
#include "../include/pmm.h"
#include "../include/paging.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// Global device
static e1000_device_t* e1000_dev = NULL;

// ============================================================================
// MMIO Register Access
// ============================================================================

static uint32_t e1000_read_reg(uint32_t reg) {
    return *((volatile uint32_t*)(e1000_dev->mmio_base + reg));
}

static void e1000_write_reg(uint32_t reg, uint32_t value) {
    *((volatile uint32_t*)(e1000_dev->mmio_base + reg)) = value;
}

// ============================================================================
// EEPROM Access
// ============================================================================

static uint16_t e1000_eeprom_read(uint8_t addr) {
    uint32_t tmp = 0;
    e1000_write_reg(E1000_REG_EERD, (1 << 0) | ((uint32_t)addr << 8));

    // Wait for read to complete
    while (!((tmp = e1000_read_reg(E1000_REG_EERD)) & (1 << 4)));

    return (uint16_t)((tmp >> 16) & 0xFFFF);
}

// ============================================================================
// MAC Address
// ============================================================================

static void e1000_read_mac_addr(void) {
    uint32_t mac_low = e1000_eeprom_read(0);
    uint32_t mac_mid = e1000_eeprom_read(1);
    uint32_t mac_high = e1000_eeprom_read(2);

    e1000_dev->mac_addr[0] = mac_low & 0xFF;
    e1000_dev->mac_addr[1] = mac_low >> 8;
    e1000_dev->mac_addr[2] = mac_mid & 0xFF;
    e1000_dev->mac_addr[3] = mac_mid >> 8;
    e1000_dev->mac_addr[4] = mac_high & 0xFF;
    e1000_dev->mac_addr[5] = mac_high >> 8;
}

void e1000_get_mac_addr(uint8_t* mac) {
    if (!e1000_dev) return;
    for (int i = 0; i < 6; i++) {
        mac[i] = e1000_dev->mac_addr[i];
    }
}

// ============================================================================
// RX/TX Ring Setup
// ============================================================================

static void e1000_rx_init(void) {
    // Allocate RX descriptors
    e1000_dev->rx_descs_phys = pmm_alloc_page();
    e1000_dev->rx_descs = (e1000_rx_desc_t*)e1000_dev->rx_descs_phys;

    // Allocate RX buffers
    e1000_dev->rx_buffers = (uint8_t**)pmm_alloc_page();

    for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
        e1000_dev->rx_buffers[i] = (uint8_t*)pmm_alloc_page();
        e1000_dev->rx_descs[i].buffer_addr = (uint64_t)e1000_dev->rx_buffers[i];
        e1000_dev->rx_descs[i].status = 0;
    }

    // Setup RX ring
    e1000_write_reg(E1000_REG_RDBAL, e1000_dev->rx_descs_phys);
    e1000_write_reg(E1000_REG_RDBAH, 0);
    e1000_write_reg(E1000_REG_RDLEN, E1000_NUM_RX_DESC * sizeof(e1000_rx_desc_t));
    e1000_write_reg(E1000_REG_RDH, 0);
    e1000_write_reg(E1000_REG_RDT, E1000_NUM_RX_DESC - 1);

    e1000_dev->rx_tail = 0;

    // Enable receiver
    e1000_write_reg(E1000_REG_RCTL,
                    E1000_RCTL_EN |
                    E1000_RCTL_UPE |
                    E1000_RCTL_MPE |
                    E1000_RCTL_BAM |
                    E1000_RCTL_BSIZE_2K);
}

static void e1000_tx_init(void) {
    // Allocate TX descriptors
    e1000_dev->tx_descs_phys = pmm_alloc_page();
    e1000_dev->tx_descs = (e1000_tx_desc_t*)e1000_dev->tx_descs_phys;

    // Allocate TX buffers
    e1000_dev->tx_buffers = (uint8_t**)pmm_alloc_page();

    for (int i = 0; i < E1000_NUM_TX_DESC; i++) {
        e1000_dev->tx_buffers[i] = (uint8_t*)pmm_alloc_page();
        e1000_dev->tx_descs[i].buffer_addr = (uint64_t)e1000_dev->tx_buffers[i];
        e1000_dev->tx_descs[i].status = E1000_DESC_STATUS_DD;
        e1000_dev->tx_descs[i].cmd = 0;
    }

    // Setup TX ring
    e1000_write_reg(E1000_REG_TDBAL, e1000_dev->tx_descs_phys);
    e1000_write_reg(E1000_REG_TDBAH, 0);
    e1000_write_reg(E1000_REG_TDLEN, E1000_NUM_TX_DESC * sizeof(e1000_tx_desc_t));
    e1000_write_reg(E1000_REG_TDH, 0);
    e1000_write_reg(E1000_REG_TDT, 0);

    e1000_dev->tx_tail = 0;

    // Enable transmitter
    e1000_write_reg(E1000_REG_TCTL,
                    E1000_TCTL_EN |
                    E1000_TCTL_PSP);
}

// ============================================================================
// Initialization
// ============================================================================

int e1000_init(void) {
    terminal_write_line("[E1000] Initializing Intel E1000 driver...");

    // Find E1000 device
    pci_device_t* pci_dev = pci_find_device(E1000_VENDOR_ID, E1000_DEVICE_ID);
    if (!pci_dev) {
        terminal_write_line("[E1000] Device not found");
        return -1;
    }

    terminal_write("[E1000] Found device at ");
    terminal_write_hex(pci_dev->bus);
    terminal_write(":");
    terminal_write_hex(pci_dev->device);
    terminal_write_line("");

    // Allocate device structure
    e1000_dev = (e1000_device_t*)pmm_alloc_page();

    // Get MMIO base address
    e1000_dev->mmio_base = pci_dev->bar[0] & ~0xF;
    e1000_dev->irq = pci_dev->interrupt_line;

    // Enable bus mastering
    pci_enable_bus_mastering(pci_dev);

    // Reset device
    e1000_write_reg(E1000_REG_CTRL, E1000_CTRL_RST);

    // Wait for reset
    for (volatile int i = 0; i < 100000; i++);

    // Read MAC address
    e1000_read_mac_addr();

    terminal_write("[E1000] MAC Address: ");
    for (int i = 0; i < 6; i++) {
        terminal_write_hex(e1000_dev->mac_addr[i]);
        if (i < 5) terminal_write(":");
    }
    terminal_write_line("");

    // Initialize RX and TX
    e1000_rx_init();
    e1000_tx_init();

    // Set link up
    uint32_t ctrl = e1000_read_reg(E1000_REG_CTRL);
    ctrl |= E1000_CTRL_SLU | E1000_CTRL_ASDE;
    e1000_write_reg(E1000_REG_CTRL, ctrl);

    e1000_dev->link_up = 1;

    terminal_write_line("[E1000] Initialization complete");

    return 0;
}

// ============================================================================
// Send Packet
// ============================================================================

int e1000_send(uint8_t* data, uint32_t len) {
    if (!e1000_dev || !e1000_dev->link_up) {
        return -1;
    }

    if (len > 2048) {
        return -1;  // Packet too large
    }

    int tail = e1000_dev->tx_tail;
    e1000_tx_desc_t* desc = &e1000_dev->tx_descs[tail];

    // Wait for descriptor to be available
    while (!(desc->status & E1000_DESC_STATUS_DD));

    // Copy data to buffer
    uint8_t* buf = e1000_dev->tx_buffers[tail];
    for (uint32_t i = 0; i < len; i++) {
        buf[i] = data[i];
    }

    // Setup descriptor
    desc->length = len;
    desc->cmd = 0x0B;  // EOP | IFCS | RS
    desc->status = 0;

    // Update tail
    e1000_dev->tx_tail = (tail + 1) % E1000_NUM_TX_DESC;
    e1000_write_reg(E1000_REG_TDT, e1000_dev->tx_tail);

    return len;
}

// ============================================================================
// Receive Packet
// ============================================================================

int e1000_receive(uint8_t* buf, uint32_t max_len) {
    if (!e1000_dev || !e1000_dev->link_up) {
        return -1;
    }

    int tail = e1000_dev->rx_tail;
    e1000_rx_desc_t* desc = &e1000_dev->rx_descs[tail];

    // Check if packet available
    if (!(desc->status & E1000_DESC_STATUS_DD)) {
        return 0;  // No packet
    }

    // Copy data from buffer
    uint32_t len = desc->length;
    if (len > max_len) len = max_len;

    uint8_t* src = e1000_dev->rx_buffers[tail];
    for (uint32_t i = 0; i < len; i++) {
        buf[i] = src[i];
    }

    // Reset descriptor
    desc->status = 0;

    // Update tail
    e1000_dev->rx_tail = (tail + 1) % E1000_NUM_RX_DESC;
    e1000_write_reg(E1000_REG_RDT, tail);

    return len;
}

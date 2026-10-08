// ============================================================================
// DMA (Direct Memory Access) Engine
// High-speed data transfer without CPU intervention
// ============================================================================

#include "../include/dma.h"
#include "../include/heap.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);

// Port I/O
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// DMA Channel Ports
static const uint16_t dma_channel_ports[8][3] = {
    {0x00, 0x01, 0x87}, // Channel 0
    {0x02, 0x03, 0x83}, // Channel 1
    {0x04, 0x05, 0x81}, // Channel 2
    {0x06, 0x07, 0x82}, // Channel 3
    {0xC0, 0xC2, 0x8F}, // Channel 4
    {0xC4, 0xC6, 0x8B}, // Channel 5
    {0xC8, 0xCA, 0x89}, // Channel 6
    {0xCC, 0xCE, 0x8A}  // Channel 7
};

// DMA Status
static uint8_t channel_status[8];
static uint8_t channel_allocated[8];

// ============================================================================
// Initialization
// ============================================================================

void dma_init(void) {
    terminal_write_line("[DMA] Initializing DMA controller...");

    // Clear all channel allocations
    for (int i = 0; i < 8; i++) {
        channel_allocated[i] = 0;
        channel_status[i] = DMA_STATUS_IDLE;
    }

    // Mask all channels
    outb(0x0A, 0x04);  // Mask channel 0
    outb(0x0A, 0x05);  // Mask channel 1
    outb(0x0A, 0x06);  // Mask channel 2
    outb(0x0A, 0x07);  // Mask channel 3

    terminal_write_line("[DMA] DMA controller initialized");
}

// ============================================================================
// Channel Management
// ============================================================================

int dma_allocate_channel(void) {
    // Skip channel 0 (reserved) and channel 4 (cascade)
    for (int i = 1; i < 8; i++) {
        if (i == 4) continue;  // Channel 4 is cascade
        if (!channel_allocated[i]) {
            channel_allocated[i] = 1;
            channel_status[i] = DMA_STATUS_IDLE;
            return i;
        }
    }
    return -1;  // No free channels
}

void dma_free_channel(uint8_t channel) {
    if (channel < 8) {
        channel_allocated[channel] = 0;
        channel_status[channel] = DMA_STATUS_IDLE;
    }
}

// ============================================================================
// DMA Transfer Setup
// ============================================================================

int dma_setup_transfer(dma_transfer_t* transfer) {
    if (!transfer || transfer->channel >= 8) {
        return -1;
    }

    uint8_t channel = transfer->channel;
    uint32_t address = (uint32_t)transfer->src_addr;
    uint16_t count = transfer->size - 1;

    // Mask channel
    outb(0x0A, 0x04 | channel);

    // Clear flip-flop
    outb(0x0C, 0xFF);

    // Set mode
    uint8_t mode = (channel & 0x03);
    if (transfer->direction == DMA_FROM_DEVICE) {
        mode |= 0x08;  // Write to memory
    } else {
        mode |= 0x04;  // Read from memory
    }
    if (transfer->mode == DMA_MODE_SINGLE) {
        mode |= 0x40;
    }
    outb(0x0B, mode);

    // Set address
    outb(dma_channel_ports[channel][0], address & 0xFF);
    outb(dma_channel_ports[channel][0], (address >> 8) & 0xFF);
    outb(dma_channel_ports[channel][2], (address >> 16) & 0xFF);

    // Set count
    outb(dma_channel_ports[channel][1], count & 0xFF);
    outb(dma_channel_ports[channel][1], (count >> 8) & 0xFF);

    // Unmask channel
    outb(0x0A, channel);

    channel_status[channel] = DMA_STATUS_IDLE;
    return 0;
}

// ============================================================================
// DMA Control
// ============================================================================

int dma_start_transfer(uint8_t channel) {
    if (channel >= 8 || !channel_allocated[channel]) {
        return -1;
    }

    channel_status[channel] = DMA_STATUS_ACTIVE;
    return 0;
}

void dma_stop_transfer(uint8_t channel) {
    if (channel < 8) {
        // Mask channel
        outb(0x0A, 0x04 | channel);
        channel_status[channel] = DMA_STATUS_IDLE;
    }
}

int dma_get_status(uint8_t channel) {
    if (channel >= 8) {
        return -1;
    }
    return channel_status[channel];
}

void dma_wait_completion(uint8_t channel) {
    if (channel >= 8) {
        return;
    }

    // Wait for transfer to complete
    while (channel_status[channel] == DMA_STATUS_ACTIVE) {
        // Check terminal count
        uint8_t status = inb(0x08);
        if (status & (1 << channel)) {
            channel_status[channel] = DMA_STATUS_COMPLETE;
            break;
        }
    }
}

// ============================================================================
// DMA Buffer Management
// ============================================================================

void* dma_alloc_buffer(uint32_t size) {
    // Allocate DMA-capable buffer (below 16MB for ISA DMA)
    return kmalloc(size);
}

void dma_free_buffer(void* buffer) {
    kfree(buffer);
}

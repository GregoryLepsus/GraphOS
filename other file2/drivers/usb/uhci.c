// ============================================================================
// USB Host Controller - UHCI Driver
// USB 1.1 Universal Host Controller Interface
// ============================================================================

#include "../include/usb.h"
#include "../include/pci.h"
#include "../include/heap.h"

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

// UHCI registers (offsets from base)
#define UHCI_USBCMD     0x00  // USB command
#define UHCI_USBSTS     0x02  // USB status
#define UHCI_USBINTR    0x04  // USB interrupt enable
#define UHCI_FRNUM      0x06  // Frame number
#define UHCI_FRBASEADD  0x08  // Frame list base address
#define UHCI_SOFMOD     0x0C  // Start of frame modify
#define UHCI_PORTSC1    0x10  // Port 1 status/control
#define UHCI_PORTSC2    0x12  // Port 2 status/control

// USB Command Register bits
#define UHCI_CMD_RUN           0x0001
#define UHCI_CMD_HCRESET       0x0002
#define UHCI_CMD_GRESET        0x0004
#define UHCI_CMD_MAXP          0x0080

// Transfer Descriptor
typedef struct {
    uint32_t link_ptr;
    uint32_t status;
    uint32_t token;
    uint32_t buffer;
} __attribute__((packed)) uhci_td_t;

// Queue Head
typedef struct {
    uint32_t head_ptr;
    uint32_t element_ptr;
} __attribute__((packed)) uhci_qh_t;

// Global USB state
static uint16_t uhci_base = 0;
static usb_device_t* device_list = NULL;
static int device_count = 0;
static uint8_t next_address = 1;

// ============================================================================
// Memory Operations
// ============================================================================

static void memcpy_uint8(uint8_t* dest, const uint8_t* src, int count) {
    for (int i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

// ============================================================================
// UHCI Controller Functions
// ============================================================================

static void uhci_reset(void) {
    // Global reset
    outw(uhci_base + UHCI_USBCMD, UHCI_CMD_GRESET);

    // Wait 10ms
    for (volatile int i = 0; i < 100000; i++);

    outw(uhci_base + UHCI_USBCMD, 0);

    // Host controller reset
    outw(uhci_base + UHCI_USBCMD, UHCI_CMD_HCRESET);

    // Wait for reset complete
    while (inw(uhci_base + UHCI_USBCMD) & UHCI_CMD_HCRESET);
}

static void uhci_start(void) {
    // Start controller
    outw(uhci_base + UHCI_USBCMD, UHCI_CMD_RUN | UHCI_CMD_MAXP);
}

static void uhci_stop(void) {
    // Stop controller
    outw(uhci_base + UHCI_USBCMD, 0);
}

// ============================================================================
// Port Operations
// ============================================================================

static void uhci_reset_port(uint16_t port_reg) {
    // Port reset
    uint16_t status = inw(uhci_base + port_reg);
    outw(uhci_base + port_reg, status | 0x0200);  // Port reset

    // Wait 50ms
    for (volatile int i = 0; i < 500000; i++);

    outw(uhci_base + port_reg, status & ~0x0200);

    // Wait for reset complete
    for (volatile int i = 0; i < 100000; i++);

    // Enable port
    status = inw(uhci_base + port_reg);
    outw(uhci_base + port_reg, status | 0x0004);
}

// ============================================================================
// Initialization
// ============================================================================

void usb_init(void) {
    terminal_write_line("[USB] Initializing USB subsystem...");

    // Find UHCI controller on PCI
    pci_device_t* uhci_dev = pci_find_class(0x0C, 0x03);  // Serial bus, USB

    if (!uhci_dev) {
        terminal_write_line("[USB] USB controller not found");
        return;
    }

    terminal_write("[USB] Found USB controller at PCI ");
    terminal_write_hex(uhci_dev->bus);
    terminal_write(":");
    terminal_write_hex(uhci_dev->device);
    terminal_write_line("");

    // Get I/O base
    uhci_base = pci_read_bar(uhci_dev, 4) & 0xFFF0;

    terminal_write("[USB] UHCI base: 0x");
    terminal_write_hex(uhci_base);
    terminal_write_line("");

    // Reset controller
    uhci_reset();

    // Allocate frame list
    uint32_t* frame_list = (uint32_t*)kmalloc(4096);
    for (int i = 0; i < 1024; i++) {
        frame_list[i] = 0x00000001;  // Terminate
    }

    // Set frame list base
    outl(uhci_base + UHCI_FRBASEADD, (uint32_t)frame_list);

    // Start controller
    uhci_start();

    device_count = 0;
    device_list = NULL;

    terminal_write_line("[USB] USB controller initialized");
}

// ============================================================================
// Device Enumeration
// ============================================================================

int usb_enumerate_devices(void) {
    if (!uhci_base) {
        return 0;
    }

    terminal_write_line("[USB] Enumerating USB devices...");

    device_count = 0;

    // Check port 1
    uint16_t port1_status = inw(uhci_base + UHCI_PORTSC1);
    if (port1_status & 0x0001) {  // Device connected
        terminal_write_line("[USB] Device detected on port 1");
        uhci_reset_port(UHCI_PORTSC1);

        // Allocate device structure
        usb_device_t* dev = (usb_device_t*)kmalloc(sizeof(usb_device_t));
        dev->address = next_address++;
        dev->port = 0;
        dev->speed = (port1_status & 0x0100) ? USB_SPEED_LOW : USB_SPEED_FULL;
        dev->next = device_list;
        device_list = dev;
        device_count++;
    }

    // Check port 2
    uint16_t port2_status = inw(uhci_base + UHCI_PORTSC2);
    if (port2_status & 0x0001) {  // Device connected
        terminal_write_line("[USB] Device detected on port 2");
        uhci_reset_port(UHCI_PORTSC2);

        usb_device_t* dev = (usb_device_t*)kmalloc(sizeof(usb_device_t));
        dev->address = next_address++;
        dev->port = 1;
        dev->speed = (port2_status & 0x0100) ? USB_SPEED_LOW : USB_SPEED_FULL;
        dev->next = device_list;
        device_list = dev;
        device_count++;
    }

    terminal_write("[USB] Found ");
    terminal_write_hex(device_count);
    terminal_write_line(" USB devices");

    return device_count;
}

// ============================================================================
// Device Access
// ============================================================================

usb_device_t* usb_get_device(uint8_t address) {
    usb_device_t* dev = device_list;

    while (dev) {
        if (dev->address == address) {
            return dev;
        }
        dev = dev->next;
    }

    return NULL;
}

// ============================================================================
// USB Transfers
// ============================================================================

int usb_control_transfer(usb_device_t* dev, uint8_t request_type,
                        uint8_t request, uint16_t value, uint16_t index,
                        void* data, uint16_t length) {
    if (!dev) {
        return -1;
    }

    // Setup packet
    struct {
        uint8_t request_type;
        uint8_t request;
        uint16_t value;
        uint16_t index;
        uint16_t length;
    } __attribute__((packed)) setup;

    setup.request_type = request_type;
    setup.request = request;
    setup.value = value;
    setup.index = index;
    setup.length = length;

    // Simplified implementation
    // Full implementation would setup TDs and QHs

    return 0;
}

int usb_bulk_transfer(usb_device_t* dev, uint8_t endpoint,
                     void* data, uint16_t length, int direction) {
    if (!dev) {
        return -1;
    }

    // Simplified implementation
    return 0;
}

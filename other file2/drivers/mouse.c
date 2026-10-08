// ============================================================================
// PS/2 Mouse Driver
// Mouse input for GUI interaction
// ============================================================================

#include "../include/mouse.h"
#include "../include/framebuffer.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);
extern void register_interrupt_handler(uint8_t n, void (*handler)(void));

// Mouse state
static mouse_state_t mouse_state;
static uint8_t mouse_cycle = 0;
static int8_t mouse_packet[3];

// PS/2 ports
#define PS2_DATA    0x60
#define PS2_COMMAND 0x64

// ============================================================================
// Port I/O
// ============================================================================

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// ============================================================================
// PS/2 Wait Functions
// ============================================================================

static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        // Wait for output buffer
        while (timeout--) {
            if ((inb(PS2_COMMAND) & 1) == 1) {
                return;
            }
        }
    } else {
        // Wait for input buffer
        while (timeout--) {
            if ((inb(PS2_COMMAND) & 2) == 0) {
                return;
            }
        }
    }
}

// ============================================================================
// PS/2 Write
// ============================================================================

static void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(PS2_COMMAND, 0xD4);
    mouse_wait(1);
    outb(PS2_DATA, data);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(PS2_DATA);
}

// ============================================================================
// Mouse Interrupt Handler
// ============================================================================

void mouse_handler(void) {
    uint8_t status = inb(PS2_COMMAND);

    if (!(status & 0x20)) {
        return;  // Not mouse data
    }

    int8_t packet = inb(PS2_DATA);

    switch (mouse_cycle) {
        case 0:
            mouse_packet[0] = packet;
            if (packet & 0x08) {  // Always 1 bit set
                mouse_cycle++;
            }
            break;

        case 1:
            mouse_packet[1] = packet;
            mouse_cycle++;
            break;

        case 2:
            mouse_packet[2] = packet;
            mouse_cycle = 0;

            // Update mouse state
            mouse_state.buttons = mouse_packet[0] & 0x07;
            mouse_state.dx = mouse_packet[1];
            mouse_state.dy = -mouse_packet[2];  // Invert Y axis

            // Update position
            mouse_state.x += mouse_state.dx;
            mouse_state.y += mouse_state.dy;

            // Clamp to screen
            if (mouse_state.x < 0) mouse_state.x = 0;
            if (mouse_state.y < 0) mouse_state.y = 0;
            if (fb) {
                if (mouse_state.x >= (int)fb->width) mouse_state.x = fb->width - 1;
                if (mouse_state.y >= (int)fb->height) mouse_state.y = fb->height - 1;
            }

            break;
    }
}

// ============================================================================
// Initialization
// ============================================================================

void mouse_init(void) {
    terminal_write_line("[Mouse] Initializing PS/2 mouse...");

    // Initialize state
    mouse_state.x = 160;
    mouse_state.y = 100;
    mouse_state.dx = 0;
    mouse_state.dy = 0;
    mouse_state.buttons = 0;
    mouse_state.visible = 1;
    mouse_cycle = 0;

    // Enable auxiliary device
    mouse_wait(1);
    outb(PS2_COMMAND, 0xA8);

    // Get status
    mouse_wait(1);
    outb(PS2_COMMAND, 0x20);
    uint8_t status = mouse_read();

    // Enable interrupts
    status |= 0x02;
    status &= ~0x20;

    // Set status
    mouse_wait(1);
    outb(PS2_COMMAND, 0x60);
    mouse_wait(1);
    outb(PS2_DATA, status);

    // Use default settings
    mouse_write(0xF6);
    mouse_read();  // Acknowledge

    // Enable data reporting
    mouse_write(0xF4);
    mouse_read();  // Acknowledge

    // Register interrupt handler (IRQ12)
    register_interrupt_handler(44, mouse_handler);

    terminal_write_line("[Mouse] PS/2 mouse initialized");
}

// ============================================================================
// Mouse Functions
// ============================================================================

void mouse_get_position(int* x, int* y) {
    if (x) *x = mouse_state.x;
    if (y) *y = mouse_state.y;
}

uint8_t mouse_get_buttons(void) {
    return mouse_state.buttons;
}

void mouse_show(void) {
    mouse_state.visible = 1;
}

void mouse_hide(void) {
    mouse_state.visible = 0;
}

void mouse_draw_cursor(void) {
    if (!mouse_state.visible || !fb) return;

    int x = mouse_state.x;
    int y = mouse_state.y;

    // Draw simple arrow cursor
    for (int i = 0; i < 16; i++) {
        fb_put_pixel(x, y + i, 15);  // White
        fb_put_pixel(x + i / 2, y + i, 15);
    }
}

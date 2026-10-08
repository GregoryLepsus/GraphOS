// ============================================================================
// VGA Graphics Driver
// Support for VGA graphics modes and drawing primitives
// ============================================================================

#include "../include/vga.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// Current mode and dimensions
static vga_mode_t current_mode;
static int screen_width;
static int screen_height;
static uint8_t* vga_memory = (uint8_t*)VGA_MEMORY;

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
// VGA Register Access
// ============================================================================

static void vga_write_regs(const uint8_t* regs) {
    // Write MISCELLANEOUS register
    outb(VGA_MISC_WRITE, *regs);
    regs++;

    // Write SEQUENCER registers
    for (int i = 0; i < 5; i++) {
        outb(VGA_SEQ_INDEX, i);
        outb(VGA_SEQ_DATA, *regs);
        regs++;
    }

    // Unlock CRTC registers
    outb(VGA_CRTC_INDEX, 0x03);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);

    // Write CRTC registers
    for (int i = 0; i < 25; i++) {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, *regs);
        regs++;
    }

    // Write GRAPHICS CONTROLLER registers
    for (int i = 0; i < 9; i++) {
        outb(VGA_GC_INDEX, i);
        outb(VGA_GC_DATA, *regs);
        regs++;
    }

    // Write ATTRIBUTE CONTROLLER registers
    for (int i = 0; i < 21; i++) {
        (void)inb(VGA_INSTAT_READ);
        outb(VGA_AC_INDEX, i);
        outb(VGA_AC_WRITE, *regs);
        regs++;
    }

    // Lock palette and unblank display
    (void)inb(VGA_INSTAT_READ);
    outb(VGA_AC_INDEX, 0x20);
}

// ============================================================================
// VGA Mode Registers
// ============================================================================

// 320x200 256-color mode (Mode 13h)
static const uint8_t g_320x200x256[] = {
    // MISC
    0x63,
    // SEQ
    0x03, 0x01, 0x0F, 0x00, 0x0E,
    // CRTC
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
    0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
    0xFF,
    // GC
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
    0xFF,
    // AC
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x41, 0x00, 0x0F, 0x00, 0x00
};

// ============================================================================
// Initialization
// ============================================================================

void vga_init(void) {
    terminal_write_line("[VGA] Initializing VGA driver...");

    current_mode = VGA_MODE_TEXT_80x25;
    screen_width = 80;
    screen_height = 25;

    terminal_write_line("[VGA] Initialization complete");
}

// ============================================================================
// Set Video Mode
// ============================================================================

void vga_set_mode(vga_mode_t mode) {
    switch (mode) {
        case VGA_MODE_GRAPHICS_320x200:
            vga_write_regs(g_320x200x256);
            screen_width = VGA_WIDTH_320;
            screen_height = VGA_HEIGHT_200;
            current_mode = mode;
            terminal_write_line("[VGA] Switched to 320x200 256-color mode");
            break;

        case VGA_MODE_TEXT_80x25:
        default:
            // Return to text mode via BIOS
            // TODO: Implement text mode restoration
            screen_width = 80;
            screen_height = 25;
            current_mode = mode;
            break;
    }
}

// ============================================================================
// Pixel Operations
// ============================================================================

void vga_plot_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) {
        return;
    }

    if (current_mode == VGA_MODE_GRAPHICS_320x200) {
        vga_memory[y * VGA_WIDTH_320 + x] = color;
    }
}

uint8_t vga_get_pixel(int x, int y) {
    if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) {
        return 0;
    }

    if (current_mode == VGA_MODE_GRAPHICS_320x200) {
        return vga_memory[y * VGA_WIDTH_320 + x];
    }

    return 0;
}

// ============================================================================
// Drawing Primitives
// ============================================================================

void vga_clear_screen(uint8_t color) {
    if (current_mode == VGA_MODE_GRAPHICS_320x200) {
        for (int i = 0; i < VGA_WIDTH_320 * VGA_HEIGHT_200; i++) {
            vga_memory[i] = color;
        }
    }
}

void vga_draw_line(int x1, int y1, int x2, int y2, uint8_t color) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        vga_plot_pixel(x1, y1, color);

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void vga_draw_rect(int x, int y, int w, int h, uint8_t color) {
    vga_draw_line(x, y, x + w - 1, y, color);           // Top
    vga_draw_line(x, y + h - 1, x + w - 1, y + h - 1, color); // Bottom
    vga_draw_line(x, y, x, y + h - 1, color);           // Left
    vga_draw_line(x + w - 1, y, x + w - 1, y + h - 1, color); // Right
}

void vga_fill_rect(int x, int y, int w, int h, uint8_t color) {
    for (int j = y; j < y + h; j++) {
        for (int i = x; i < x + w; i++) {
            vga_plot_pixel(i, j, color);
        }
    }
}

void vga_draw_circle(int cx, int cy, int radius, uint8_t color) {
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (x <= y) {
        vga_plot_pixel(cx + x, cy + y, color);
        vga_plot_pixel(cx - x, cy + y, color);
        vga_plot_pixel(cx + x, cy - y, color);
        vga_plot_pixel(cx - x, cy - y, color);
        vga_plot_pixel(cx + y, cy + x, color);
        vga_plot_pixel(cx - y, cy + x, color);
        vga_plot_pixel(cx + y, cy - x, color);
        vga_plot_pixel(cx - y, cy - x, color);

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

// ============================================================================
// Palette Management
// ============================================================================

void vga_set_palette(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    outb(VGA_DAC_WRITE_INDEX, index);
    outb(VGA_DAC_DATA, r);
    outb(VGA_DAC_DATA, g);
    outb(VGA_DAC_DATA, b);
}

// ============================================================================
// Framebuffer Graphics Layer
// Abstract framebuffer interface for graphics operations
// ============================================================================

#include "../include/framebuffer.h"
#include "../include/vga.h"
#include "../include/heap.h"

// Global framebuffer instance
framebuffer_t* fb = NULL;

// ============================================================================
// Memory Operations
// ============================================================================

static void memset_uint8(uint8_t* dest, uint8_t value, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        dest[i] = value;
    }
}

static void memcpy_uint8(uint8_t* dest, const uint8_t* src, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

// ============================================================================
// Initialization
// ============================================================================

void fb_init(uint32_t width, uint32_t height, uint8_t bpp) {
    // Allocate framebuffer structure
    fb = (framebuffer_t*)kmalloc(sizeof(framebuffer_t));

    fb->width = width;
    fb->height = height;
    fb->bpp = bpp;
    fb->pitch = width * (bpp / 8);
    fb->buffer = (uint8_t*)VGA_MEMORY;  // VGA memory at 0xA0000
    fb->backbuffer = NULL;
    fb->double_buffer_enabled = 0;
}

void fb_enable_double_buffer(int enable) {
    if (enable && !fb->backbuffer) {
        // Allocate back buffer
        uint32_t buffer_size = fb->height * fb->pitch;
        fb->backbuffer = (uint8_t*)kmalloc(buffer_size);
        fb->double_buffer_enabled = 1;
    } else if (!enable && fb->backbuffer) {
        // Free back buffer
        kfree(fb->backbuffer);
        fb->backbuffer = NULL;
        fb->double_buffer_enabled = 0;
    }
}

// ============================================================================
// Buffer Operations
// ============================================================================

void fb_clear(uint32_t color) {
    uint8_t* target = fb->double_buffer_enabled ? fb->backbuffer : fb->buffer;
    uint32_t buffer_size = fb->height * fb->pitch;

    if (fb->bpp == 8) {
        memset_uint8(target, (uint8_t)color, buffer_size);
    }
}

void fb_swap_buffers(void) {
    if (fb->double_buffer_enabled && fb->backbuffer) {
        uint32_t buffer_size = fb->height * fb->pitch;
        memcpy_uint8(fb->buffer, fb->backbuffer, buffer_size);
    }
}

// ============================================================================
// Pixel Operations
// ============================================================================

void fb_put_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= (int)fb->width || y < 0 || y >= (int)fb->height) {
        return;
    }

    uint8_t* target = fb->double_buffer_enabled ? fb->backbuffer : fb->buffer;

    if (fb->bpp == 8) {
        target[y * fb->pitch + x] = (uint8_t)color;
    }
}

uint32_t fb_get_pixel(int x, int y) {
    if (x < 0 || x >= (int)fb->width || y < 0 || y >= (int)fb->height) {
        return 0;
    }

    uint8_t* target = fb->double_buffer_enabled ? fb->backbuffer : fb->buffer;

    if (fb->bpp == 8) {
        return target[y * fb->pitch + x];
    }

    return 0;
}

// ============================================================================
// Drawing Primitives
// ============================================================================

void fb_draw_line(int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        fb_put_pixel(x1, y1, color);

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

void fb_draw_rect(int x, int y, int w, int h, uint32_t color) {
    fb_draw_line(x, y, x + w - 1, y, color);
    fb_draw_line(x, y + h - 1, x + w - 1, y + h - 1, color);
    fb_draw_line(x, y, x, y + h - 1, color);
    fb_draw_line(x + w - 1, y, x + w - 1, y + h - 1, color);
}

void fb_fill_rect(int x, int y, int w, int h, uint32_t color) {
    for (int j = y; j < y + h; j++) {
        for (int i = x; i < x + w; i++) {
            fb_put_pixel(i, j, color);
        }
    }
}

void fb_draw_circle(int cx, int cy, int radius, uint32_t color) {
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (x <= y) {
        fb_put_pixel(cx + x, cy + y, color);
        fb_put_pixel(cx - x, cy + y, color);
        fb_put_pixel(cx + x, cy - y, color);
        fb_put_pixel(cx - x, cy - y, color);
        fb_put_pixel(cx + y, cy + x, color);
        fb_put_pixel(cx - y, cy + x, color);
        fb_put_pixel(cx + y, cy - x, color);
        fb_put_pixel(cx - y, cy - x, color);

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

void fb_fill_circle(int cx, int cy, int radius, uint32_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                fb_put_pixel(cx + x, cy + y, color);
            }
        }
    }
}

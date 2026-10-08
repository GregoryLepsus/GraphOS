// ============================================================================
// Font Rendering
// Bitmap font rendering for GUI text
// ============================================================================

#include "../include/font.h"
#include "../include/framebuffer.h"

// External functions
extern void terminal_write_line(const char* str);

// ============================================================================
// Built-in 8x16 Font Data
// ============================================================================

// Simple ASCII font (first 128 characters)
static const uint8_t font_8x16_data[] = {
    // Character 0-31: Control characters (use space)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // ... (repeat for chars 1-31)
    // Character 32: Space
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // Character 33: !
    0x00, 0x00, 0x18, 0x3C, 0x3C, 0x3C, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
    // Character 34: "
    0x00, 0x66, 0x66, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // Character 35: #
    0x00, 0x00, 0x00, 0x6C, 0x6C, 0xFE, 0x6C, 0x6C, 0x6C, 0xFE, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00,
    // ... (more characters)
    // Character 65: A
    0x00, 0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0xC6, 0xC6, 0xC6, 0xC6, 0x00, 0x00, 0x00, 0x00,
};

// Default font instance
font_t font_8x16 = {
    .width = 8,
    .height = 16,
    .glyphs = font_8x16_data,
    .glyph_count = 128
};

// ============================================================================
// Initialization
// ============================================================================

void font_init(void) {
    terminal_write_line("[Font] Font rendering initialized");
}

// ============================================================================
// String Length
// ============================================================================

static int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

// ============================================================================
// Draw Character
// ============================================================================

void font_draw_char(font_t* font, char c, int x, int y, uint32_t fg, uint32_t bg) {
    if (!font || !fb) return;

    // Get glyph data
    uint8_t char_index = (uint8_t)c;
    if (char_index >= font->glyph_count) {
        char_index = 0;  // Use null character for unknown chars
    }

    const uint8_t* glyph = &font->glyphs[char_index * font->height];

    // Draw glyph
    for (int row = 0; row < font->height; row++) {
        uint8_t line = glyph[row];
        for (int col = 0; col < font->width; col++) {
            if (line & (0x80 >> col)) {
                fb_put_pixel(x + col, y + row, fg);
            } else if (bg != 0xFFFFFFFF) {  // Transparent background if 0xFFFFFFFF
                fb_put_pixel(x + col, y + row, bg);
            }
        }
    }
}

// ============================================================================
// Draw String
// ============================================================================

void font_draw_string(font_t* font, const char* str, int x, int y, uint32_t fg, uint32_t bg) {
    if (!font || !str) return;

    int cursor_x = x;
    int cursor_y = y;

    while (*str) {
        if (*str == '\n') {
            cursor_x = x;
            cursor_y += font->height;
        } else if (*str == '\t') {
            cursor_x += font->width * 4;  // Tab = 4 spaces
        } else {
            font_draw_char(font, *str, cursor_x, cursor_y, fg, bg);
            cursor_x += font->width;
        }
        str++;
    }
}

// ============================================================================
// Measure String Width
// ============================================================================

int font_measure_string(font_t* font, const char* str) {
    if (!font || !str) return 0;

    int width = 0;
    int max_width = 0;

    while (*str) {
        if (*str == '\n') {
            if (width > max_width) {
                max_width = width;
            }
            width = 0;
        } else if (*str == '\t') {
            width += font->width * 4;
        } else {
            width += font->width;
        }
        str++;
    }

    if (width > max_width) {
        max_width = width;
    }

    return max_width;
}

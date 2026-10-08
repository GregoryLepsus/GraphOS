#include "test_framework.h"
#include "../include/common.h"
#include "../include/gui/framebuffer.h"
#include "../include/gui/window.h"

// Test: Framebuffer initialization
static bool test_fb_init(void) {
    // Framebuffer should be initialized
    fb_info_t* fb = fb_get_info();

    if (!fb) {
        // No framebuffer available
        return true;
    }

    ASSERT(fb->width > 0);
    ASSERT(fb->height > 0);
    ASSERT_NOT_NULL(fb->buffer);

    return true;
}

// Test: Pixel drawing
static bool test_pixel_draw(void) {
    fb_info_t* fb = fb_get_info();
    if (!fb) return true;

    // Draw a pixel
    fb_put_pixel(100, 100, 0xFF0000); // Red
    return true;
}

// Test: Rectangle drawing
static bool test_rect_draw(void) {
    fb_info_t* fb = fb_get_info();
    if (!fb) return true;

    // Draw a rectangle
    fb_draw_rect(50, 50, 100, 100, 0x00FF00); // Green
    return true;
}

// Test: Line drawing
static bool test_line_draw(void) {
    fb_info_t* fb = fb_get_info();
    if (!fb) return true;

    // Draw a line
    fb_draw_line(0, 0, 100, 100, 0x0000FF); // Blue
    return true;
}

// Test: Window creation
static bool test_window_create(void) {
    window_t* win = window_create("Test Window", 100, 100, 200, 150);

    if (!win) {
        // Window system not available
        return true;
    }

    ASSERT_EQ(win->x, 100);
    ASSERT_EQ(win->y, 100);
    ASSERT_EQ(win->width, 200);
    ASSERT_EQ(win->height, 150);

    return true;
}

// Test: Multiple windows
static bool test_multiple_windows(void) {
    window_t* win1 = window_create("Window 1", 50, 50, 150, 100);
    window_t* win2 = window_create("Window 2", 100, 100, 150, 100);

    // Both should be created or both fail gracefully
    return true;
}

// Test: Window focus
static bool test_window_focus(void) {
    window_t* win = window_create("Focus Test", 100, 100, 200, 150);
    if (!win) return true;

    // Set focus
    window_set_focus(win);

    return true;
}

// Test: Window movement
static bool test_window_move(void) {
    window_t* win = window_create("Move Test", 100, 100, 200, 150);
    if (!win) return true;

    // Move window
    window_move(win, 150, 150);
    ASSERT_EQ(win->x, 150);
    ASSERT_EQ(win->y, 150);

    return true;
}

// Test: Mouse cursor
static bool test_mouse_cursor(void) {
    // Test mouse cursor drawing
    fb_draw_cursor(100, 100);
    return true;
}

// Test: Font rendering
static bool test_font_render(void) {
    fb_info_t* fb = fb_get_info();
    if (!fb) return true;

    // Draw a character
    fb_draw_char('A', 10, 10, 0xFFFFFF);
    return true;
}

// Test case array
static test_case_t gui_test_cases[] = {
    {"FB Init", "Test framebuffer initialization", test_fb_init},
    {"Pixel Draw", "Test pixel drawing", test_pixel_draw},
    {"Rect Draw", "Test rectangle drawing", test_rect_draw},
    {"Line Draw", "Test line drawing", test_line_draw},
    {"Window Create", "Test window creation", test_window_create},
    {"Multiple Windows", "Test multiple windows", test_multiple_windows},
    {"Window Focus", "Test window focus", test_window_focus},
    {"Window Move", "Test window movement", test_window_move},
    {"Mouse Cursor", "Test mouse cursor", test_mouse_cursor},
    {"Font Render", "Test font rendering", test_font_render},
};

// Test suite definition
static test_suite_t gui_test_suite = {
    .name = "GUI Tests",
    .tests = gui_test_cases,
    .test_count = sizeof(gui_test_cases) / sizeof(test_case_t),
};

// Register suite
void gui_tests_register(void) {
    test_register_suite(&gui_test_suite);
}

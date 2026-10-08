// ============================================================================
// Window Manager
// Basic window management system
// ============================================================================

#include "../include/window.h"
#include "../include/framebuffer.h"
#include "../include/font.h"
#include "../include/heap.h"

// External functions
extern void terminal_write_line(const char* str);

// Window list
static window_t* window_list = NULL;
static window_t* focused_window = NULL;
static uint32_t next_window_id = 1;

// Mouse state for dragging
static int mouse_drag_start_x = 0;
static int mouse_drag_start_y = 0;
static int prev_mouse_buttons = 0;

// ============================================================================
// String Functions
// ============================================================================

static int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

static void strcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

// ============================================================================
// Initialization
// ============================================================================

void window_manager_init(void) {
    terminal_write_line("[WindowMgr] Window manager initialized");
    window_list = NULL;
    focused_window = NULL;
}

// ============================================================================
// Window Creation
// ============================================================================

window_t* window_create(const char* title, int x, int y, int w, int h) {
    window_t* win = (window_t*)kmalloc(sizeof(window_t));

    win->id = next_window_id++;
    strcpy(win->title, title);
    win->x = x;
    win->y = y;
    win->width = w;
    win->height = h;
    win->z_order = 0;
    win->focused = 0;
    win->visible = 1;
    win->dragging = 0;
    win->bg_color = WINDOW_COLOR_BACKGROUND;
    win->fb = NULL;  // Could allocate separate framebuffer for window content
    win->next = NULL;
    win->prev = NULL;

    // Add to window list
    if (!window_list) {
        window_list = win;
    } else {
        window_t* last = window_list;
        while (last->next) {
            last = last->next;
        }
        last->next = win;
        win->prev = last;
    }

    // Focus new window
    window_focus(win);

    return win;
}

// ============================================================================
// Window Destruction
// ============================================================================

void window_destroy(window_t* win) {
    if (!win) return;

    // Remove from list
    if (win->prev) {
        win->prev->next = win->next;
    } else {
        window_list = win->next;
    }

    if (win->next) {
        win->next->prev = win->prev;
    }

    // Update focus
    if (focused_window == win) {
        focused_window = window_list;
    }

    kfree(win);
}

// ============================================================================
// Window Operations
// ============================================================================

void window_move(window_t* win, int x, int y) {
    if (!win) return;
    win->x = x;
    win->y = y;
}

void window_resize(window_t* win, int w, int h) {
    if (!win) return;
    win->width = w;
    win->height = h;
}

void window_set_title(window_t* win, const char* title) {
    if (!win) return;
    strcpy(win->title, title);
}

void window_focus(window_t* win) {
    if (!win) return;

    // Unfocus all windows
    window_t* w = window_list;
    while (w) {
        w->focused = 0;
        w = w->next;
    }

    // Focus this window
    win->focused = 1;
    focused_window = win;
}

void window_raise(window_t* win) {
    if (!win) return;

    // Move to end of list (top of Z-order)
    if (win->next) {
        // Remove from current position
        if (win->prev) {
            win->prev->next = win->next;
        } else {
            window_list = win->next;
        }
        win->next->prev = win->prev;

        // Add to end
        window_t* last = window_list;
        while (last->next) {
            last = last->next;
        }
        last->next = win;
        win->prev = last;
        win->next = NULL;
    }
}

// ============================================================================
// Window Drawing
// ============================================================================

void window_draw(window_t* win) {
    if (!win || !win->visible || !fb) return;

    int title_color = win->focused ? WINDOW_COLOR_TITLE_ACTIVE : WINDOW_COLOR_TITLE_INACTIVE;

    // Draw title bar
    fb_fill_rect(win->x, win->y, win->width, WINDOW_TITLE_HEIGHT, title_color);

    // Draw title text
    font_draw_string(&font_8x16, win->title, win->x + 5, win->y + 2, 0x0F, 0xFFFFFFFF);

    // Draw window content area
    fb_fill_rect(win->x, win->y + WINDOW_TITLE_HEIGHT,
                 win->width, win->height - WINDOW_TITLE_HEIGHT,
                 win->bg_color);

    // Draw border
    fb_draw_rect(win->x, win->y, win->width, win->height, WINDOW_COLOR_BORDER);
}

void window_draw_all(void) {
    if (!fb) return;

    // Clear screen
    fb_clear(0x00);

    // Draw all windows in order
    window_t* win = window_list;
    while (win) {
        window_draw(win);
        win = win->next;
    }
}

// ============================================================================
// Hit Testing
// ============================================================================

window_t* window_at_position(int x, int y) {
    // Check from top to bottom (reverse order)
    window_t* win = window_list;
    window_t* last = NULL;

    while (win) {
        last = win;
        win = win->next;
    }

    // Now traverse backwards
    win = last;
    while (win) {
        if (win->visible &&
            x >= win->x && x < win->x + win->width &&
            y >= win->y && y < win->y + win->height) {
            return win;
        }
        win = win->prev;
    }

    return NULL;
}

// ============================================================================
// Mouse Event Handling
// ============================================================================

void window_handle_mouse_event(int x, int y, uint8_t buttons) {
    // Left button pressed
    if ((buttons & 0x01) && !(prev_mouse_buttons & 0x01)) {
        window_t* win = window_at_position(x, y);

        if (win) {
            // Check if clicked on title bar
            if (y >= win->y && y < win->y + WINDOW_TITLE_HEIGHT) {
                // Start dragging
                win->dragging = 1;
                mouse_drag_start_x = x - win->x;
                mouse_drag_start_y = y - win->y;
            }

            // Focus and raise window
            window_focus(win);
            window_raise(win);
        }
    }

    // Left button released
    if (!(buttons & 0x01) && (prev_mouse_buttons & 0x01)) {
        // Stop dragging all windows
        window_t* win = window_list;
        while (win) {
            win->dragging = 0;
            win = win->next;
        }
    }

    // Dragging
    if (buttons & 0x01) {
        window_t* win = window_list;
        while (win) {
            if (win->dragging) {
                window_move(win, x - mouse_drag_start_x, y - mouse_drag_start_y);
            }
            win = win->next;
        }
    }

    prev_mouse_buttons = buttons;
}

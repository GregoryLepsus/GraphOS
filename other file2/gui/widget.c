// ============================================================================
// GUI Widgets
// Basic widget library for GUI applications
// ============================================================================

#include "../include/widget.h"
#include "../include/framebuffer.h"
#include "../include/font.h"
#include "../include/heap.h"

// ============================================================================
// String Functions
// ============================================================================

static void strcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

// ============================================================================
// Widget Drawing Functions
// ============================================================================

static void button_draw(widget_t* widget, window_t* win) {
    if (!widget || !win) return;

    button_data_t* data = (button_data_t*)widget->data;
    uint32_t bg_color = data->pressed ? WIDGET_COLOR_BUTTON_PRESSED : WIDGET_COLOR_BUTTON_BG;

    int abs_x = win->x + widget->x;
    int abs_y = win->y + WINDOW_TITLE_HEIGHT + widget->y;

    // Draw button background
    fb_fill_rect(abs_x, abs_y, widget->width, widget->height, bg_color);

    // Draw border
    fb_draw_rect(abs_x, abs_y, widget->width, widget->height, WIDGET_COLOR_BORDER);

    // Draw label (centered)
    int text_width = font_measure_string(&font_8x16, data->label);
    int text_x = abs_x + (widget->width - text_width) / 2;
    int text_y = abs_y + (widget->height - font_8x16.height) / 2;

    font_draw_string(&font_8x16, data->label, text_x, text_y,
                     WIDGET_COLOR_BUTTON_FG, 0xFFFFFFFF);
}

static void label_draw(widget_t* widget, window_t* win) {
    if (!widget || !win) return;

    label_data_t* data = (label_data_t*)widget->data;

    int abs_x = win->x + widget->x;
    int abs_y = win->y + WINDOW_TITLE_HEIGHT + widget->y;

    font_draw_string(&font_8x16, data->text, abs_x, abs_y,
                     data->color, 0xFFFFFFFF);
}

static void textbox_draw(widget_t* widget, window_t* win) {
    if (!widget || !win) return;

    textbox_data_t* data = (textbox_data_t*)widget->data;

    int abs_x = win->x + widget->x;
    int abs_y = win->y + WINDOW_TITLE_HEIGHT + widget->y;

    // Draw textbox background
    fb_fill_rect(abs_x, abs_y, widget->width, widget->height, WIDGET_COLOR_TEXTBOX_BG);

    // Draw border
    fb_draw_rect(abs_x, abs_y, widget->width, widget->height, WIDGET_COLOR_BORDER);

    // Draw text
    font_draw_string(&font_8x16, data->text, abs_x + 4, abs_y + 4,
                     WIDGET_COLOR_TEXTBOX_FG, 0xFFFFFFFF);
}

static void checkbox_draw(widget_t* widget, window_t* win) {
    if (!widget || !win) return;

    checkbox_data_t* data = (checkbox_data_t*)widget->data;

    int abs_x = win->x + widget->x;
    int abs_y = win->y + WINDOW_TITLE_HEIGHT + widget->y;

    // Draw checkbox box (16x16)
    fb_fill_rect(abs_x, abs_y, 16, 16, WIDGET_COLOR_TEXTBOX_BG);
    fb_draw_rect(abs_x, abs_y, 16, 16, WIDGET_COLOR_BORDER);

    // Draw check mark if checked
    if (data->checked) {
        fb_draw_line(abs_x + 3, abs_y + 8, abs_x + 6, abs_y + 11, WIDGET_COLOR_TEXTBOX_FG);
        fb_draw_line(abs_x + 6, abs_y + 11, abs_x + 13, abs_y + 4, WIDGET_COLOR_TEXTBOX_FG);
    }

    // Draw label
    font_draw_string(&font_8x16, data->label, abs_x + 20, abs_y,
                     WIDGET_COLOR_LABEL_FG, 0xFFFFFFFF);
}

// ============================================================================
// Button Widget
// ============================================================================

widget_t* widget_create_button(const char* label, int x, int y, int w, int h, void (*on_click)(widget_t*)) {
    widget_t* widget = (widget_t*)kmalloc(sizeof(widget_t));
    button_data_t* data = (button_data_t*)kmalloc(sizeof(button_data_t));

    widget->type = WIDGET_BUTTON;
    widget->x = x;
    widget->y = y;
    widget->width = w;
    widget->height = h;
    widget->visible = 1;
    widget->enabled = 1;
    widget->hovered = 0;
    widget->data = data;
    widget->on_click = on_click;
    widget->on_draw = button_draw;
    widget->next = NULL;

    strcpy(data->label, label);
    data->pressed = 0;

    return widget;
}

// ============================================================================
// Label Widget
// ============================================================================

widget_t* widget_create_label(const char* text, int x, int y, uint32_t color) {
    widget_t* widget = (widget_t*)kmalloc(sizeof(widget_t));
    label_data_t* data = (label_data_t*)kmalloc(sizeof(label_data_t));

    widget->type = WIDGET_LABEL;
    widget->x = x;
    widget->y = y;
    widget->width = font_measure_string(&font_8x16, text);
    widget->height = font_8x16.height;
    widget->visible = 1;
    widget->enabled = 1;
    widget->hovered = 0;
    widget->data = data;
    widget->on_click = NULL;
    widget->on_draw = label_draw;
    widget->next = NULL;

    strcpy(data->text, text);
    data->color = color;

    return widget;
}

// ============================================================================
// Textbox Widget
// ============================================================================

widget_t* widget_create_textbox(int x, int y, int w, int h) {
    widget_t* widget = (widget_t*)kmalloc(sizeof(widget_t));
    textbox_data_t* data = (textbox_data_t*)kmalloc(sizeof(textbox_data_t));

    widget->type = WIDGET_TEXTBOX;
    widget->x = x;
    widget->y = y;
    widget->width = w;
    widget->height = h;
    widget->visible = 1;
    widget->enabled = 1;
    widget->hovered = 0;
    widget->data = data;
    widget->on_click = NULL;
    widget->on_draw = textbox_draw;
    widget->next = NULL;

    data->text[0] = '\0';
    data->cursor_pos = 0;
    data->max_length = 255;

    return widget;
}

// ============================================================================
// Checkbox Widget
// ============================================================================

widget_t* widget_create_checkbox(const char* label, int x, int y, int checked) {
    widget_t* widget = (widget_t*)kmalloc(sizeof(widget_t));
    checkbox_data_t* data = (checkbox_data_t*)kmalloc(sizeof(checkbox_data_t));

    widget->type = WIDGET_CHECKBOX;
    widget->x = x;
    widget->y = y;
    widget->width = 16 + 20 + font_measure_string(&font_8x16, label);
    widget->height = 16;
    widget->visible = 1;
    widget->enabled = 1;
    widget->hovered = 0;
    widget->data = data;
    widget->on_click = NULL;
    widget->on_draw = checkbox_draw;
    widget->next = NULL;

    strcpy(data->label, label);
    data->checked = checked;

    return widget;
}

// ============================================================================
// Widget Operations
// ============================================================================

void widget_destroy(widget_t* widget) {
    if (!widget) return;

    if (widget->data) {
        kfree(widget->data);
    }

    kfree(widget);
}

void widget_draw(widget_t* widget, window_t* win) {
    if (!widget || !widget->visible || !widget->on_draw) return;

    widget->on_draw(widget, win);
}

int widget_handle_click(widget_t* widget, int x, int y) {
    if (!widget || !widget->enabled) return 0;

    // Check if click is within widget bounds
    if (x >= widget->x && x < widget->x + widget->width &&
        y >= widget->y && y < widget->y + widget->height) {

        // Handle specific widget types
        if (widget->type == WIDGET_CHECKBOX) {
            checkbox_data_t* data = (checkbox_data_t*)widget->data;
            data->checked = !data->checked;
        }

        // Call custom handler
        if (widget->on_click) {
            widget->on_click(widget);
        }

        return 1;
    }

    return 0;
}

widget_t* widget_at_position(widget_t* list, int x, int y) {
    widget_t* widget = list;

    while (widget) {
        if (widget->visible &&
            x >= widget->x && x < widget->x + widget->width &&
            y >= widget->y && y < widget->y + widget->height) {
            return widget;
        }
        widget = widget->next;
    }

    return NULL;
}

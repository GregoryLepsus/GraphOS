// ============================================================================
// USB HID (Human Interface Device) Driver
// Support for USB keyboards, mice, and other input devices
// ============================================================================

#include "../../include/usb.h"
#include "../../include/heap.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);

// HID Class-Specific Requests
#define HID_GET_REPORT      0x01
#define HID_GET_IDLE        0x02
#define HID_GET_PROTOCOL    0x03
#define HID_SET_REPORT      0x09
#define HID_SET_IDLE        0x0A
#define HID_SET_PROTOCOL    0x0B

// HID Descriptor Types
#define HID_DESC_HID        0x21
#define HID_DESC_REPORT     0x22
#define HID_DESC_PHYSICAL   0x23

// HID Protocol
#define HID_PROTOCOL_BOOT   0
#define HID_PROTOCOL_REPORT 1

// HID Boot Protocol Keyboard Report
typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
} __attribute__((packed)) hid_keyboard_report_t;

// HID Boot Protocol Mouse Report
typedef struct {
    uint8_t buttons;
    int8_t x;
    int8_t y;
} __attribute__((packed)) hid_mouse_report_t;

// ============================================================================
// HID Initialization
// ============================================================================

int usb_hid_init(usb_device_t* dev) {
    if (!dev) {
        return -1;
    }

    terminal_write_line("[USB HID] Initializing HID device...");

    // Set boot protocol
    int result = usb_control_transfer(dev, 0x21, HID_SET_PROTOCOL,
                                     HID_PROTOCOL_BOOT, 0, NULL, 0);
    if (result < 0) {
        terminal_write_line("[USB HID] Failed to set boot protocol");
        return -1;
    }

    // Set idle (no repeat)
    usb_control_transfer(dev, 0x21, HID_SET_IDLE, 0, 0, NULL, 0);

    terminal_write_line("[USB HID] HID device initialized");
    return 0;
}

// ============================================================================
// Keyboard Functions
// ============================================================================

int usb_hid_read_keyboard(usb_device_t* dev, hid_keyboard_report_t* report) {
    if (!dev || !report) {
        return -1;
    }

    return usb_bulk_transfer(dev, 0x81, report, sizeof(hid_keyboard_report_t), 1);
}

// ============================================================================
// Mouse Functions
// ============================================================================

int usb_hid_read_mouse(usb_device_t* dev, hid_mouse_report_t* report) {
    if (!dev || !report) {
        return -1;
    }

    return usb_bulk_transfer(dev, 0x81, report, sizeof(hid_mouse_report_t), 1);
}

// ============================================================================
// Hardware Abstraction Layer (HAL)
// Unified device interface for driver management
// ============================================================================

#include "../include/hal.h"
#include "../include/heap.h"

// External functions
extern void terminal_write(const char* str);
extern void terminal_write_line(const char* str);
extern void terminal_write_hex(uint32_t value);

// Device list
static hal_device_t* device_list = NULL;
static uint32_t device_count = 0;

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
// Initialization
// ============================================================================

void hal_init(void) {
    terminal_write_line("[HAL] Initializing Hardware Abstraction Layer...");
    device_list = NULL;
    device_count = 0;
    terminal_write_line("[HAL] HAL initialized");
}

// ============================================================================
// Device Registration
// ============================================================================

int hal_register_device(hal_device_t* device) {
    if (!device) {
        return -1;
    }

    // Initialize device
    if (device->init) {
        int result = device->init(device);
        if (result < 0) {
            terminal_write("[HAL] Failed to initialize device: ");
            terminal_write_line(device->name);
            return -1;
        }
    }

    // Add to device list
    device->next = device_list;
    device_list = device;
    device_count++;

    device->flags |= HAL_FLAG_INITIALIZED | HAL_FLAG_READY;

    terminal_write("[HAL] Registered device: ");
    terminal_write(device->name);
    terminal_write(" (vendor: 0x");
    terminal_write_hex(device->vendor_id);
    terminal_write(", device: 0x");
    terminal_write_hex(device->device_id);
    terminal_write_line(")");

    return 0;
}

// ============================================================================
// Device Unregistration
// ============================================================================

int hal_unregister_device(hal_device_t* device) {
    if (!device) {
        return -1;
    }

    // Shutdown device
    if (device->shutdown) {
        device->shutdown(device);
    }

    // Remove from list
    hal_device_t* prev = NULL;
    hal_device_t* curr = device_list;

    while (curr) {
        if (curr == device) {
            if (prev) {
                prev->next = curr->next;
            } else {
                device_list = curr->next;
            }
            device_count--;
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }

    return -1;
}

// ============================================================================
// Device Lookup
// ============================================================================

hal_device_t* hal_find_device(uint32_t vendor_id, uint32_t device_id) {
    hal_device_t* dev = device_list;

    while (dev) {
        if (dev->vendor_id == vendor_id && dev->device_id == device_id) {
            return dev;
        }
        dev = dev->next;
    }

    return NULL;
}

hal_device_t* hal_find_device_by_type(hal_device_type_t type) {
    hal_device_t* dev = device_list;

    while (dev) {
        if (dev->type == type) {
            return dev;
        }
        dev = dev->next;
    }

    return NULL;
}

// ============================================================================
// Device Enumeration
// ============================================================================

int hal_device_count(void) {
    return device_count;
}

hal_device_t* hal_get_device(int index) {
    if (index < 0 || index >= (int)device_count) {
        return NULL;
    }

    hal_device_t* dev = device_list;
    int i = 0;

    while (dev && i < index) {
        dev = dev->next;
        i++;
    }

    return dev;
}

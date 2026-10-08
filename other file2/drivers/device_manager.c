// ============================================================================
// Device Manager
// Central device management and enumeration
// ============================================================================

#include "../include/device_manager.h"
#include "../include/heap.h"

static device_entry_t* devices = NULL;
static int device_count = 0;

// ============================================================================
// Initialization
// ============================================================================

void devmgr_init(void) {
    device_count = 0;
}

// ============================================================================
// Device Registration
// ============================================================================

int devmgr_register(const char* name, device_type_t type, void* driver) {
    device_entry_t* dev = (device_entry_t*)kmalloc(sizeof(device_entry_t));
    if (!dev) return -1;

    // Copy name
    int i;
    for (i = 0; i < 31 && name[i]; i++) {
        dev->name[i] = name[i];
    }
    dev->name[i] = '\0';

    dev->type = type;
    dev->driver = driver;
    dev->flags = 0;
    dev->next = devices;

    devices = dev;
    device_count++;

    return 0;
}

int devmgr_unregister(const char* name) {
    device_entry_t* prev = NULL;
    device_entry_t* curr = devices;

    while (curr) {
        int match = 1;
        for (int i = 0; i < 32; i++) {
            if (curr->name[i] != name[i]) {
                match = 0;
                break;
            }
            if (name[i] == '\0') break;
        }

        if (match) {
            if (prev) {
                prev->next = curr->next;
            } else {
                devices = curr->next;
            }
            kfree(curr);
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

device_entry_t* devmgr_find(const char* name) {
    device_entry_t* curr = devices;

    while (curr) {
        int match = 1;
        for (int i = 0; i < 32; i++) {
            if (curr->name[i] != name[i]) {
                match = 0;
                break;
            }
            if (name[i] == '\0') break;
        }

        if (match) {
            return curr;
        }

        curr = curr->next;
    }

    return NULL;
}

int devmgr_list(device_entry_t** list, int max) {
    if (!list || max <= 0) return 0;

    device_entry_t* curr = devices;
    int count = 0;

    while (curr && count < max) {
        list[count++] = curr;
        curr = curr->next;
    }

    return count;
}

#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <stdint.h>

// Device Types
typedef enum {
    DEVTYPE_BLOCK,
    DEVTYPE_CHAR,
    DEVTYPE_NETWORK,
    DEVTYPE_INPUT,
    DEVTYPE_AUDIO,
    DEVTYPE_VIDEO
} device_type_t;

// Device Entry
typedef struct device_entry {
    char name[32];
    device_type_t type;
    void* driver;
    uint32_t flags;
    struct device_entry* next;
} device_entry_t;

// Device Manager Functions
void devmgr_init(void);
int devmgr_register(const char* name, device_type_t type, void* driver);
int devmgr_unregister(const char* name);
device_entry_t* devmgr_find(const char* name);
int devmgr_list(device_entry_t** list, int max);

#endif // DEVICE_MANAGER_H

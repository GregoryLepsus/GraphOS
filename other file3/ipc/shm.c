// ============================================================================
// Shared Memory IPC
// Memory regions shared between processes
// ============================================================================

#include "../include/shm.h"
#include "../include/heap.h"
#include "../include/pmm.h"

static shm_region_t* shm_regions = NULL;
static shm_attachment_t* shm_attachments = NULL;
static uint32_t next_shm_id = 1;

// ============================================================================
// Shared Memory Management
// ============================================================================

shm_region_t* shm_create(uint32_t size, uint32_t permissions) {
    if (size == 0) return NULL;

    // Allocate region structure
    shm_region_t* region = (shm_region_t*)kmalloc(sizeof(shm_region_t));
    if (!region) return NULL;

    // Allocate shared memory
    uint32_t pages = (size + 4095) / 4096;
    void* memory = kmalloc(pages * 4096);
    if (!memory) {
        kfree(region);
        return NULL;
    }

    // Initialize region
    region->id = next_shm_id++;
    region->size = size;
    region->kernel_addr = memory;
    region->ref_count = 0;
    region->permissions = permissions;
    region->owner_pid = 0;  // TODO: Get current process PID
    region->next = shm_regions;

    shm_regions = region;

    return region;
}

void* shm_attach(uint32_t shm_id, uint32_t pid) {
    // Find region
    shm_region_t* region = shm_find(shm_id);
    if (!region) return NULL;

    // Create attachment
    shm_attachment_t* attach = (shm_attachment_t*)kmalloc(sizeof(shm_attachment_t));
    if (!attach) return NULL;

    attach->shm_id = shm_id;
    attach->pid = pid;
    attach->address = region->kernel_addr;
    attach->next = shm_attachments;

    shm_attachments = attach;
    region->ref_count++;

    return region->kernel_addr;
}

int shm_detach(void* address, uint32_t pid) {
    if (!address) return -1;

    // Find attachment
    shm_attachment_t* prev = NULL;
    shm_attachment_t* attach = shm_attachments;

    while (attach) {
        if (attach->address == address && attach->pid == pid) {
            // Remove attachment
            if (prev) {
                prev->next = attach->next;
            } else {
                shm_attachments = attach->next;
            }

            // Decrement ref count
            shm_region_t* region = shm_find(attach->shm_id);
            if (region) {
                region->ref_count--;
            }

            kfree(attach);
            return 0;
        }
        prev = attach;
        attach = attach->next;
    }

    return -1;
}

int shm_destroy(uint32_t shm_id) {
    // Find region
    shm_region_t* prev = NULL;
    shm_region_t* region = shm_regions;

    while (region) {
        if (region->id == shm_id) {
            // Check if still attached
            if (region->ref_count > 0) {
                return -1;  // Still in use
            }

            // Remove from list
            if (prev) {
                prev->next = region->next;
            } else {
                shm_regions = region->next;
            }

            // Free memory
            kfree(region->kernel_addr);
            kfree(region);

            return 0;
        }
        prev = region;
        region = region->next;
    }

    return -1;
}

shm_region_t* shm_find(uint32_t shm_id) {
    shm_region_t* region = shm_regions;

    while (region) {
        if (region->id == shm_id) {
            return region;
        }
        region = region->next;
    }

    return NULL;
}
